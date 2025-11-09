#ifndef GAME_TESTS_H
#define GAME_TESTS_H

void TestsEventHandler(struct mg_connection* c, int ev, void* ev_data);

#endif /* GAME_TESTS_H */

#ifdef GAME_TESTS_IMPLEMENTATION

void TestsEventHandler(struct mg_connection* c, int ev, void* ev_data) {
	switch (ev) {
		case MG_EV_WS_OPEN:
			printf("OPEN\n");
			break;
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

#endif /* GAME_TESTS_IMPLEMENTATION */
