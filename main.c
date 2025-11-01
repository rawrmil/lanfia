#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
#undef NOB_IMPLEMENTATION

#define FLAG_IMPLEMENTATION
#include "flag.h"
#undef FLAG_IMPLEMENTATION

#define BYTERW_IMPLEMENTATION
#include "byterw.h"
#undef BYTERW_IMPLEMENTATION

#include "mongoose.h"

// --- APP ---

typedef struct AppConfig {
	int port;
	char* web_dir;
} AppConfig;

AppConfig config;

void AppParseFlags(int argc, char** argv) {
	mg_log_set(MG_LL_NONE);

	bool* f_help = flag_bool("help", 0, "help");
	uint64_t* f_ll = flag_uint64("log-level", 0, "none, error, info, debug, verbose (0, 1, 2, 3, 4)");
	uint64_t* f_port = flag_uint64("port", 6969, "port for the server");
	char** f_web_dir = flag_str("webdir", "./web", "directory for the server");

	if (!flag_parse(argc, argv)) {
    flag_print_options(stdout);
		flag_print_error(stderr);
		exit(1);
	}

	if (*f_help) {
    flag_print_options(stdout);
		exit(0);
	}

	config.web_dir = *f_web_dir;
	config.port = *f_port;
	mg_log_set(*f_ll);
}


// --- USERS ---

typedef struct GameUser {
	struct mg_connection* c;
} GameUser;

int users_count = 0;

// --- PLAYERS ---

typedef struct GamePlayer {
	struct mg_connection* c;
	Nob_String_Builder username;
} GamePlayer;

typedef struct GamePlayers {
	GamePlayer* items;
	size_t count;
	size_t capacity;
} GamePlayers; // nob.h dynamic array

GamePlayers players;


/*
	GAME SERVER MESSAGES:
		Server:
			[ GSMT_INFO_VIEWERS | uint32_t(4b) ]
			- information about viewer count
*/

// Game Server Message Types

#define GSMT \
	X(GSMT_INFO_VIEWERS) \
	X(GSMT_LAST_)\

#define X(name_) name_,
enum GameServerMessageTypes { GSMT };
#undef X

#define X(name_) #name_,
const char *gsmt_names[] = { GSMT };
#undef X

// Game Client Message Types

#define GCMT \
	X(GCMT_LOBBY_JOIN) \
	X(GCMT_LAST_)\

#define X(name_) name_,
enum GameClientMessageTypes { GCMT };
#undef X

#define X(name_) #name_,
const char *gcmt_names[] = { GCMT };
#undef X

// JS

Nob_String_Builder gmt_js;

void GameMessageTypesGenerateJS() {
	nob_sb_append_cstr(&gmt_js, "const GSMT = {\n");
	for (int i = 0; i < GSMT_LAST_; i++) {
		nob_sb_appendf(&gmt_js, "\t%s: %d,\n", gsmt_names[i]+strlen("GSMT_"), i);
	}
	nob_sb_append_cstr(&gmt_js, "};\n");
	nob_sb_append_cstr(&gmt_js, "const GCMT = {\n");
	for (int i = 0; i < GCMT_LAST_; i++) {
		nob_sb_appendf(&gmt_js, "\t%s: %d,\n", gcmt_names[i]+strlen("GCMT_"), i);
	}
	nob_sb_append_cstr(&gmt_js, "};");
	nob_sb_append_null(&gmt_js);
}

void GameUsersUpdate(struct mg_mgr* mgr) {
	uint32_t count = users_count > players.count ? users_count-players.count : 0;
	ByteWriter bw = {0};
	ByteWriterU8(&bw, GSMT_INFO_VIEWERS);
	ByteWriterU32(&bw, count);
	MG_INFO(("viewers: %d\n", count));
	for (struct mg_connection* c = mgr->conns; c != NULL; c = c->next) {
		mg_ws_send(c, bw.sb.items, bw.sb.count, WEBSOCKET_OP_BINARY);
	}
	ByteWriterFree(bw);
}

void GameUserAdd(struct mg_connection* c) {
	GameUser* user = calloc(1, sizeof(*user));
	assert(user != NULL);
	user->c = c;
	users_count++;
	GameUsersUpdate(c->mgr);
	MG_INFO(("users: %d\n", users_count));
}

void GameUserRemove(struct mg_connection* c) {
	// TODO: IF PHASE == LOBBY
	nob_da_foreach(GamePlayer, p, &players) {
		if (p->c == c) {
			nob_da_remove_unordered(&players, p - players.items);
		}
	}
	free(c->fn_data);
	users_count--;
	GameUsersUpdate(c->mgr);
	MG_INFO(("users: %d\n", users_count));
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
		struct mg_http_serve_opts opts = { .root_dir = config.web_dir };
		mg_http_serve_dir(c, hm, &opts);
	}
}

void HandleWSClose(struct mg_connection* c, void* ev_data) {
	GameUserRemove(c);
}

bool HandleGCMTLobbyJoin(struct mg_connection* c, ByteReader* br) {
	mg_hexdump(br->sv.data, br->sv.count);
	bool result = true;
	uint32_t n;
	if (!ByteReaderU32(br, &n))
		nob_return_defer(false);
	Nob_String_Builder username = ByteReaderSBAlloc(br, n); // TODO: free
	nob_da_foreach(GamePlayer, p, &players) {
		if (p->c == c) {
			// TODO: already connected
			nob_return_defer(false);
		}
	}
	GamePlayer player = {0};
	player.c = c;
	player.username = username; // TODO: check username
	nob_da_append(&players, player);
	GameUsersUpdate(c->mgr);
defer:
	nob_sb_free(username);
	return result;
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
			HandleGCMTLobbyJoin(c, &br);
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

	printf("log_level: %d\n", mg_log_level);
	printf("config.web_dir: %s\n", config.web_dir);
	printf("config.port: %d\n", config.port);

	struct mg_mgr mgr;
	mg_mgr_init(&mgr);

	char addrstr[32];
	snprintf(addrstr, sizeof(addrstr), "http://0.0.0.0:%d", config.port);

	mg_http_listen(&mgr, addrstr, EventHandler, NULL);

	while (true) {
		mg_mgr_poll(&mgr, 1000);
	}

	// Closing
	mg_mgr_free(&mgr);
	printf("Server closed.\n");

	return 0;
}
