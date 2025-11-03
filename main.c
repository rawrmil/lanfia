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

#define BYTERW_IMPLEMENTATION
#include "byterw.h"
#undef BYTERW_IMPLEMENTATION

// --- GAME PARTS ---

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


// --- APP ---

typedef struct Flags {
	bool* help;
	uint64_t* ll;
	uint64_t* port;
	char** web_dir;
} Flags;

Flags flags;

void AppParseFlags(int argc, char** argv) {
	mg_log_set(MG_LL_NONE);

	flags.help = flag_bool("help", 0, "help");
	flags.ll = flag_uint64("log-level", 0, "none, error, info, debug, verbose (0, 1, 2, 3, 4)");
	flags.port = flag_uint64("port", 6969, "port for the server");
	flags.web_dir = flag_str("webdir", "./web", "directory for the server");

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

void HandleWSClose(struct mg_connection* c, void* ev_data) {
	GameUserRemove(c);
}


void HandleWSMessage(struct mg_connection* c, void* ev_data) {
	struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
	ByteReader br = {0};
	br.sv.count = wm->data.len;
	br.sv.data = wm->data.buf;
	uint8_t gcmt;
	if (!ByteReaderU8(&br, &gcmt)) return;
	switch (gcmt) {
		case GCMT_LOBBY_JOIN:
			HandleClientLobbyJoin(c, &br);
			break;
	}
}

void EventHandler(struct mg_connection* c, int ev, void* ev_data) {
	switch (ev) {
		case MG_EV_HTTP_MSG:
			HandleHTTPMessage(c, ev_data);
			break;
		case MG_EV_WS_MSG:
			HandleWSMessage(c, ev_data);
			break;
		case MG_EV_CLOSE:
			if (c->is_websocket) { HandleWSClose(c, ev_data); }
			break;
		case MG_EV_POLL:
			break;
	}
}

// --- MAIN ---

int main(int argc, char* argv[]) {


	GameMessageTypesGenerateJS();

	AppParseFlags(argc, argv);

	Nob_String_Builder ip_sb = {0};
	nob_sb_append_cstr(&ip_sb, "http://");
	GetIpSB(&ip_sb);
	nob_sb_appendf(&ip_sb, ":%d", *flags.port);
	printf("Address: %.*s\n", ip_sb.count, ip_sb.items);

	Nob_String_Builder ip_qrcode = {0}; // as a bitmap
	int bitmap_size;
	if (GetQRCodeBitmap(&ip_qrcode, ip_sb.items, &bitmap_size)) {
		PrintBitmapSmall(ip_qrcode.items, bitmap_size, 0);
	}
	nob_sb_free(ip_sb);
	nob_sb_free(ip_qrcode);

	printf("log_level: %d\n", mg_log_level);
	printf("flags.web_dir: %s\n", *flags.web_dir);
	printf("flags.port: %d\n", *flags.port);

	struct mg_mgr mgr;
	mg_mgr_init(&mgr);

	char addrstr[32];
	snprintf(addrstr, sizeof(addrstr), "http://0.0.0.0:%d", *flags.port);

	mg_http_listen(&mgr, addrstr, EventHandler, NULL);

	while (true) {
		mg_mgr_poll(&mgr, 1000);
	}

	// Closing
	mg_mgr_free(&mgr);
	printf("Server closed.\n");

	return 0;
}
