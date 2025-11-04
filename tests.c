#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"
#undef NOB_IMPLEMENTATION

#include "mongoose.h"

#ifdef __linux__
#define BINARY "lanfia"
#elif __APPLE__
#define BINARY "lanfia"
#elif __WIN32
#define BINARY "lanfia.exe"
#else
#error "Unsupported platform."
#endif

#define PORT "6969"

void ws_fn(struct mg_connection* c, int ev, void* ev_data) {
	switch (ev) {
		case MG_EV_WS_MSG:
			printf("MSG\n");
			break;
		case MG_EV_CLOSE:
			printf("CLOSE\n");
			break;
		case MG_EV_POLL:
			break;
	}
}

int counter = 0;

int main(int argc, char** argv) {

	mg_log_set(MG_LL_INFO);

	Cmd cmd = {0};
	cmd_append(&cmd, "./build/"BINARY, "-port", PORT);
	Proc proc = cmd_run_async(cmd);

	struct mg_mgr mgr;
	mg_mgr_init(&mgr);
	struct mg_connection* c[32];
	bool done = false;
	c[0] = mg_ws_connect(&mgr, "ws://localhost:6969/ws", ws_fn, &done, NULL);
	while (done == false) {
		mg_mgr_poll(&mgr, 100);
		printf("count: %d\n", counter);
		counter++;
	}

	while(1);
	return 0;
}
