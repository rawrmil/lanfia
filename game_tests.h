#ifndef GAME_TESTS_H
#define GAME_TESTS_H

#include "nob.h"
#include "byterw.h"
#include "game_enums.h"

typedef struct TestMsg {
	ByteWriter msg;
	bool is_check;
	size_t conn_index;
	Nob_String_Builder desc;
} TestMsg;

typedef struct TestBatch {
	TestMsg* items;
	size_t count;
	size_t capacity;
	size_t index;
} TestBatch; // nob.h dynamic array

extern struct mg_connection* test_conns[32];

void TestsInit();
void TestsEventHandler(struct mg_connection* c, int ev, void* ev_data);

#endif /* GAME_TESTS_H */

#ifdef GAME_TESTS_IMPLEMENTATION

struct mg_connection* test_conns[32];
bool tests_done;

TestBatch batch;

void TestMsgAdd(bool is_check, int conn_index, ByteWriter msg, char* desc) {
	TestMsg test_msg = {0};
	test_msg.msg = msg;
	test_msg.is_check = is_check;
	test_msg.conn_index = conn_index;
	test_msg.desc = (Nob_String_Builder){0};
	if (desc)
		nob_sb_append_cstr(&test_msg.desc, desc);
	nob_da_append(&batch, test_msg);
}

void TestsInit() {
	TestMsgAdd(false, 0, ByteWriterBuild((ByteWriter){0}, BU8, GCMT_LOBBY_JOIN, BSN, 7, "player1"), NULL);
	TestMsgAdd(false, 1, ByteWriterBuild((ByteWriter){0}, BU8, GCMT_LOBBY_JOIN, BSN, 7, "player2"), NULL);
	TestMsgAdd(false, 2, ByteWriterBuild((ByteWriter){0}, BU8, GCMT_LOBBY_JOIN, BSN, 7, "player3"), NULL);
	TestMsgAdd(false, 3, ByteWriterBuild((ByteWriter){0}, BU8, GCMT_LOBBY_JOIN, BSN, 7, "player4"), "Added last player");
}

void TestsStep(struct mg_connection* c) {
	if (tests_done) return;
	if (batch.index >= batch.count) {
		printf("TESTS: end\n");
		tests_done = true;
		return;
	}
	TestMsg test_msg = batch.items[batch.index];
	if (test_msg.desc.count > 0) {
		printf("[ %d/%d ]: ", batch.index, batch.count);
		printf("'%.*s'\n", test_msg.desc.count, test_msg.desc.items);
	}
	mg_ws_send(test_conns[test_msg.conn_index], test_msg.msg.sb.items, test_msg.msg.sb.count, WEBSOCKET_OP_BINARY);
	batch.index++;
}

void TestsEventHandler(struct mg_connection* c, int ev, void* ev_data) {
	if (!c->is_client)
		return;
	switch (ev) {
		case MG_EV_WS_OPEN:
			TestsStep(c);
			break;
		case MG_EV_WS_MSG:
			TestsStep(c);
			break;
		case MG_EV_CLOSE:
			break;
		case MG_EV_POLL:
			break;
	}
}

#endif /* GAME_TESTS_IMPLEMENTATION */
