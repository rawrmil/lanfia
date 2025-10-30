#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
#undef NOB_IMPLEMENTATION

#define FLAG_IMPLEMENTATION
#include "flag.h"
#undef FLAG_IMPLEMENTATION

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

// --- EVENTS ---

void HandleHTTPMessage(struct mg_connection* c, void* ev_data) {
	struct mg_http_message* hm = (struct mg_http_message*)ev_data;
	if (mg_strcmp(hm->uri, mg_str("/ws")) == 0) {
		mg_ws_upgrade(c, hm, NULL);
		mg_ws_send(c, "TEST", 4, WEBSOCKET_OP_TEXT);
		return;
	}
	if (!strncmp(hm->method.buf, "GET", 3)) {
			struct mg_http_serve_opts opts = { .root_dir = config.web_dir };
			mg_http_serve_dir(c, hm, &opts);
	}
}


void HandleWSMessage(struct mg_connection* c, void* ev_data) {
	struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
	printf("%.*s\n", wm->data.len, wm->data.buf);
}

void HandleClose(struct mg_connection* c, void* ev_data) {
	// ...
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
			HandleClose(c, ev_data);
			break;
		case MG_EV_POLL:
			break;
	}
}

// --- MAIN ---

int main(int argc, char* argv[]) {
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
