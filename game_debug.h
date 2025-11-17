#ifndef GAME_DEBUG_H
#define GAME_DEBUG_H

#include "nob.h"
#include "byterw.h"
#include "game_enums.h"

typedef struct ClientMsg {
	ByteWriter msg;
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

void ClientMsgAdd(int conn_index, ByteWriter msg, char* desc) {
	ClientMsg cm = {0};
	cm.msg = msg;
	cm.conn_index = conn_index;
	cm.desc = (Nob_String_Builder){0};
	if (desc)
		nob_sb_append_cstr(&cm.desc, desc);
	nob_da_append(&batch, cm);
}

void MsgSeq6PlayersConnect() {
	ClientMsgAdd(0, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 6, "mafia1"),    NULL);
	ClientMsgAdd(1, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 6, "mafia2"),    NULL);
	ClientMsgAdd(2, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 6, "doctor"),    NULL);
	ClientMsgAdd(3, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "cop"),       NULL);
	ClientMsgAdd(4, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 9, "villager1"), NULL);
	ClientMsgAdd(5, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 9, "villager2"), NULL);
}

void MsgSeq6PlayersReady() {
	ClientMsgAdd(0, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 6, "mafia1"),    NULL);
	ClientMsgAdd(0, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN), NULL); // Error check
	ClientMsgAdd(0, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_READY, BU8, 1), NULL);
	ClientMsgAdd(1, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 6, "mafia2"),    NULL);
	ClientMsgAdd(1, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_READY, BU8, 1), NULL);
	ClientMsgAdd(2, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 6, "doctor"),    NULL);
	ClientMsgAdd(2, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_READY, BU8, 1), NULL);
	ClientMsgAdd(3, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "cop"),       NULL);
	ClientMsgAdd(3, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_READY, BU8, 1), NULL);
	ClientMsgAdd(4, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 9, "villager1"), NULL);
	ClientMsgAdd(4, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_READY, BU8, 1), NULL);
	ClientMsgAdd(5, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 9, "villager2"), NULL);
	ClientMsgAdd(5, ByteWriterBuild(NULL, BU8, GCMT_LOBBY_READY, BU8, 1), NULL);
}

void MsgSeqInit(int state) {
	switch (state) {
		case 1: MsgSeq6PlayersConnect(); break;
		case 2: MsgSeq6PlayersReady(); break;
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
	mg_ws_send(debug_conns[cm.conn_index], cm.msg.sb.items, cm.msg.sb.count, WEBSOCKET_OP_BINARY);
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
