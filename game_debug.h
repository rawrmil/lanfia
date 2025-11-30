#ifndef GAME_DEBUG_H
#define GAME_DEBUG_H

#include "nob.h"
#include "binary_rw.h"
#include "game_enums.h"

typedef struct ClientMsg {
	BWriter msg;
	size_t conn_index;
	Nob_String_Builder desc;
} ClientMsg;

typedef struct MsgSeq {
	ClientMsg* items;
	size_t count;
	size_t capacity;
	size_t index;
} MsgSeq; // nob.h dynamic array

extern struct mg_connection* debug_conns[32];

void MsgSeqInit(int state);
void DebugEventHandler(struct mg_connection* c, int ev, void* ev_data);

#endif /* GAME_DEBUG_H */

#ifdef GAME_DEBUG_IMPLEMENTATION

struct mg_connection* debug_conns[32];
bool seq_done;

MsgSeq batch;

void ClientMsgAdd(int conn_index, BWriter msg) {
	ClientMsg cm = {0};
	cm.msg = msg;
	cm.conn_index = conn_index;
	cm.desc = (Nob_String_Builder){0};
	nob_da_append(&batch, cm);
}

void MsgJoinNPlayers(int n) {
	for (int i = 0; i < n; i++) {
		ClientMsgAdd(i, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, nob_temp_sprintf("%03d", i)));
	}
}
void MsgReadyNPlayers(int n) {
	for (int i = 0; i < n; i++) {
		ClientMsgAdd(i, BWriterAppend(NULL, BU8, GCMT_LOBBY_READY, BU8, 1));
	}
}
void MsgReadyNextNPlayers(int n) {
	for (int i = 0; i < n; i++) {
		ClientMsgAdd(i, BWriterAppend(NULL, BU8, GCMT_READY_NEXT, BU8, 1));
	}
}

void MsgSeq6PlayersConnect() {
	MsgJoinNPlayers(6);
}

void MsgSeq6PlayersReady() {
	MsgJoinNPlayers(6);
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 0, ""));
	MsgReadyNPlayers(6);
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN)); // Error check
}

void MsgSeqGameStart() {
	MsgJoinNPlayers(6);
	MsgReadyNPlayers(6);
}

void MsgSeq4Players() {
	MsgJoinNPlayers(4);
}

void MsgSeq10PlayersReady() {
	MsgJoinNPlayers(10);
	MsgReadyNPlayers(10);
}

void MsgSeq32PlayersReady() {
	MsgJoinNPlayers(32);
	MsgReadyNPlayers(32);
}

void MsgSeq6PlayersReadyNext() {
	MsgJoinNPlayers(6);
	MsgReadyNPlayers(6);
	MsgReadyNextNPlayers(6);
}

void MsgSeqInit(int state) {
	switch (state) {
		case 1: MsgSeq6PlayersConnect(); break;
		case 2: MsgSeq6PlayersReady(); break;
		case 3: MsgSeqGameStart(); break;
		case 4: MsgSeq4Players(); break;
		case 5: MsgSeq10PlayersReady(); break;
		case 6: MsgSeq32PlayersReady(); break;
		case 7: MsgSeq6PlayersReadyNext(); break;
	}
}

void SeqStep(struct mg_connection* c) {
	if (seq_done) return;
	if (batch.index >= batch.count) {
		printf("SEQUENCE: end\n");
		seq_done = true;
		return;
	}
	ClientMsg cm = batch.items[batch.index];
	if (debug_conns[cm.conn_index] != c) return;
	if (cm.desc.count > 0) {
		printf("[ %d/%d ]: ", batch.index, batch.count);
		printf("'%.*s'\n", cm.desc.count, cm.desc.items);
	}
	mg_ws_send(debug_conns[cm.conn_index], cm.msg.items, cm.msg.count, WEBSOCKET_OP_BINARY);
	batch.index++;
}

void DebugEventHandler(struct mg_connection* c, int ev, void* ev_data) {
	if (!c->is_client)
		return;
	switch (ev) {
		case MG_EV_WS_OPEN:
			SeqStep(c);
			break;
		case MG_EV_WS_MSG:
			SeqStep(c);
			break;
		case MG_EV_CLOSE:
			break;
		case MG_EV_POLL:
			break;
	}
}

#endif /* GAME_DEBUG_IMPLEMENTATION */
