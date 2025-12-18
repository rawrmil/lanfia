#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mongoose.h"

#define NOB_IMPLEMENTATION
#include "nob.h"
#undef NOB_IMPLEMENTATION

#define FLAG_IMPLEMENTATION
#include "flag.h"
#undef FLAG_IMPLEMENTATION

#define BINARY_RW_IMPLEMENTATION
#include "binary_rw.h"
#undef BINARY_RW_IMPLEMENTATION

// --- GAME PARTS ---

#define GAME_DEBUG_IMPLEMENTATION
#include "game_debug.h"
#undef GAME_DEBUG_IMPLEMENTATION

#define GAME_LOGIC_IMPLEMENTATION
#include "game_logic.h"
#undef GAME_LOGIC_IMPLEMENTATION

#define GAME_ENUMS_IMPLEMENTATION
#include "game_enums.h"
#undef GAME_ENUMS_IMPLEMENTATION

#define GAME_GETIP_IMPLEMENTATION
#include "game_getip.h"
#undef GAME_GETIP_IMPLEMENTATION

// --- GLOBALS ---

#define MAX_TIME 100.0

// --- APP ---

typedef struct Flags {
	bool* help;
	uint64_t* ll;
	uint64_t* port;
	char** web_dir;
	uint64_t* state;
} Flags;

Flags flags;

void AppParseFlags(int argc, char** argv) {
	mg_log_set(MG_LL_NONE);

	flags.help = flag_bool("help", 0, "help");
	flags.ll = flag_uint64("log-level", 0, "none, error, info, debug, verbose (0, 1, 2, 3, 4)");
	flags.port = flag_uint64("port", 6969, "port for the server");
	flags.web_dir = flag_str("webdir", "./web", "directory for the server");
	flags.state = flag_uint64("state", 0, "debug state of the game");

	if (!flag_parse(argc, argv)) {
    flag_print_options(stdout);
		flag_print_error(stderr);
		exit(1);
	}

	if (*flags.help) {
    flag_print_options(stdout);
		exit(0);
	}

	mg_log_set(*flags.ll);
}

// --- EVENTS ---

void HandleHTTPMessage(struct mg_connection* c, void* ev_data) {
	struct mg_http_message* hm = (struct mg_http_message*)ev_data;
	if (mg_strcmp(hm->uri, mg_str("/ws")) == 0) {
		mg_ws_upgrade(c, hm, NULL);
		GameUserAdd(c);
		return;
	}
	if (!strncmp(hm->method.buf, "GET", 3)) {
		if (mg_strcmp(hm->uri, mg_str("/gmt.js")) == 0) {
			mg_http_reply(c, 200, "", gmt_js.items);
		}
		struct mg_http_serve_opts opts = { .root_dir = *flags.web_dir };
		mg_http_serve_dir(c, hm, &opts);
	}
}

void HandleWSMessage(struct mg_connection* c, void* ev_data) {
	struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
	BReader br = {0};
	br.count = wm->data.len;
	br.data = wm->data.buf;
	uint8_t gcmt;
	//mg_hexdump(br.data, br.count);
	if (!BReadU8(&br, &gcmt)) return;
	switch (gcmt) {
		case GCMT_LOBBY_JOIN:
			HandleClientLobbyJoin(c, &br);
			break;
		case GCMT_LOBBY_LEAVE:
			HandleClientLobbyLeave(c, &br);
			break;
		case GCMT_LOBBY_READY:
			HandleClientLobbyReady(c, &br);
			break;
		case GCMT_READY_NEXT:
			HandleClientReadyNext(c, &br);
			break;
		case GCMT_POLL:
			HandleClientLobbyPoll(c, &br);
			break;
		case GCMT_DEBUG_SET_ROLE:
			if (!game.debug) { break; }
			game.manual_roles = true;
			GamePlayer* p = GameGetPlayer(c);
			if (p == NULL) { break; }
			uint8_t r;
			if (!BReadU8(&br, &r)) { break; }
			p->role = r;
			break;
	}
}

void EventHandler(struct mg_connection* c, int ev, void* ev_data) {
	if (c->is_client)
		return;
	switch (ev) {
		case MG_EV_HTTP_MSG:
			HandleHTTPMessage(c, ev_data);
			break;
		case MG_EV_WS_OPEN:
			HandleClientConnect(c);
			break;
		case MG_EV_WS_MSG:
			HandleWSMessage(c, ev_data);
			break;
		case MG_EV_CLOSE:
			if (c->is_websocket) {
				HandleClientDisconnect(c);
			}
			break;
		case MG_EV_POLL:
			break;
	}
}

// --- MAIN ---

int main(int argc, char* argv[]) {
	srand(nob_nanos_since_unspecified_epoch());

	AppParseFlags(argc, argv);

	Nob_String_Builder ip_sb = {0};
	nob_sb_append_cstr(&ip_sb, "http://");
	GetIpSB(&ip_sb);
	nob_sb_appendf(&ip_sb, ":%lu", *flags.port);
	printf("Address: %.*s\n", (int)ip_sb.count, ip_sb.items);

	Nob_String_Builder ip_qrcode = {0}; // as a bitmap
	int bitmap_size;
	if (GetQRCodeBitmap(&ip_qrcode, ip_sb.items, &bitmap_size)) {
		PrintBitmapSmall(ip_qrcode.items, bitmap_size, 0);
	}
	nob_sb_free(ip_sb);
	nob_sb_free(ip_qrcode);

	printf("log_level: %d\n", mg_log_level);
	printf("flags.web_dir: %s\n", *flags.web_dir);
	printf("flags.port: %lu\n", *flags.port);

	struct mg_mgr mgr;
	mg_mgr_init(&mgr);

	char addrstr[32];
	snprintf(addrstr, sizeof(addrstr), "http://0.0.0.0:%lu", *flags.port);

	mg_http_listen(&mgr, addrstr, EventHandler, NULL);

	if (*flags.state != 0) {
		game.debug = true;
		MsgSeqInit(*flags.state);
		snprintf(addrstr, sizeof(addrstr), "ws://0.0.0.0:%lu/ws", *flags.port);
		for (size_t i = 0; i < sizeof(debug_conns)/sizeof(*debug_conns); i++)
			debug_conns[i] = mg_ws_connect(&mgr, addrstr, DebugEventHandler, NULL, NULL);
	}

	GameMessageTypesGenerateJS();

	while (1) {
		mg_mgr_poll(&mgr, 200);
		// TODO: move this shit
		double curr = (double)nob_nanos_since_unspecified_epoch() / (double)NOB_NANOS_PER_SEC;
		static double last_poll = 0.0;
		if (last_poll == 0.0) { last_poll = curr; }
		uint64_t sec_rem = (uint32_t)(MAX_TIME - (curr - game.last_state_s));
		if (curr > last_poll + 1.0) {
			last_poll = curr;
			if (game.state != GS_LOBBY){
				bw_temp.count = 0;
				BWriterAppend(&bw_temp,
					BU8, GSMT_TIMER,
					BU32, sec_rem);
				GameSendAction(mgr.conns, nob_sb_to_sv(bw_temp), -1);
			}
		}
		if (curr > game.last_state_s + MAX_TIME) {
			GameChangeState(mgr.conns);
		}
	}

	// Closing
	mg_mgr_free(&mgr);
	printf("Server closed.\n");

	return 0;
}
