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
void MsgVote(int n, int v) {
	for (int i = 0; i < n; i++) {
		ClientMsgAdd(i, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, v));
	}
}

// Sequences

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
	MsgReadyNextNPlayers(5);
}

void MsgSeq6FixedRoles() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "maf"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	MsgReadyNPlayers(6);
}

void MsgSeq6FixedRolesReadyNext() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "maf"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	MsgReadyNPlayers(6);
	MsgReadyNextNPlayers(6);
}

void MsgSeq6FixedRolesReadyNextDay() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "maf"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	MsgReadyNPlayers(6);
	MsgReadyNextNPlayers(6);
	MsgReadyNextNPlayers(6);
	MsgReadyNextNPlayers(5);
}
void MsgSeq6MafiaTest() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf1"));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf2"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	MsgReadyNPlayers(7);
	MsgReadyNextNPlayers(7);
	MsgReadyNextNPlayers(5);
}

void MsgSeq6MafiaTest2() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf1"));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf2"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	MsgReadyNPlayers(7);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 1));
	MsgReadyNextNPlayers(7);
	MsgReadyNextNPlayers(7);
	MsgReadyNextNPlayers(5);
}

void MsgSeq6VoteTest() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf1"));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf2"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	MsgReadyNPlayers(7);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 3));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 3));
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 0));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 0));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2)); // Dead one
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2));
	MsgReadyNextNPlayers(7);
}

void MsgSeqMafiaWon() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf1"));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf2"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	MsgReadyNPlayers(7);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 0));
	MsgReadyNextNPlayers(7);
	MsgVote(7, 1);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2));
	MsgReadyNextNPlayers(7);
	MsgVote(7, 3);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 4));
	MsgReadyNextNPlayers(7);
}

void MsgSeqTownWon() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf1"));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf2"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	MsgReadyNPlayers(7);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 5));
	MsgReadyNextNPlayers(7);
	MsgVote(7, 4);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 6));
	MsgReadyNextNPlayers(7);
}

void MsgSeqManiacWon() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf1"));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf2"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	MsgReadyNPlayers(7);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 5));
	MsgReadyNextNPlayers(7);
	MsgVote(7, 0);
	MsgReadyNextNPlayers(7);
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 6));
	MsgReadyNextNPlayers(7);
	MsgVote(7, 1);
	MsgReadyNextNPlayers(7);
	MsgReadyNextNPlayers(7);
	MsgVote(7, 2);
	MsgReadyNextNPlayers(7);
	MsgReadyNextNPlayers(7);
	MsgVote(7, 3);
	MsgReadyNextNPlayers(7);
}

void MsgSeqMafiaIgnore() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf1"));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 4, "maf2"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(6, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	MsgReadyNPlayers(7);
	MsgReadyNextNPlayers(7);
	MsgReadyNextNPlayers(7);
}

void MsgSeqDoctorHeal() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "maf"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	MsgReadyNPlayers(6);
	MsgReadyNextNPlayers(6);
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 3));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 3));
	MsgReadyNextNPlayers(5);
}

void MsgSeqSerifCheck() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "maf"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	MsgReadyNPlayers(6);
	MsgReadyNextNPlayers(6);
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 3));
	MsgReadyNextNPlayers(5);
}

void MsgSeqEscortCheck() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "maf"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	MsgReadyNPlayers(6);
	MsgReadyNextNPlayers(6);
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 1));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2));
	MsgReadyNextNPlayers(6);
}

void MsgSeqGameManiac() {
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "vil"));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "doc"));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "ser"));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "esc"));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "maf"));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_LOBBY_JOIN, BSN, 3, "man"));
	ClientMsgAdd(0, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_TOWNSMAN));
	ClientMsgAdd(1, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_DOCTOR));
	ClientMsgAdd(2, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_SERIF));
	ClientMsgAdd(3, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_ESCORT));
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MAFIA));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_DEBUG_SET_ROLE, BU8, GRT_MANIAC));
	MsgReadyNPlayers(6);
	MsgReadyNextNPlayers(6);
	ClientMsgAdd(4, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 1));
	ClientMsgAdd(5, BWriterAppend(NULL, BU8, GCMT_POLL, BU32, 2));
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
		case 8: MsgSeq6FixedRoles(); break;
		case 9: MsgSeq6FixedRolesReadyNext(); break;
		case 10: MsgSeq6FixedRolesReadyNextDay(); break;
		case 11: MsgSeq6MafiaTest(); break;
		case 12: MsgSeq6MafiaTest2(); break;
		case 13: MsgSeq6VoteTest(); break;
		case 14: MsgSeqMafiaWon(); break;
		case 15: MsgSeqTownWon(); break;
		case 16: MsgSeqManiacWon(); break;
		case 17: MsgSeqMafiaIgnore(); break;
		case 18: MsgSeqDoctorHeal(); break;
		case 19: MsgSeqSerifCheck(); break;
		case 20: MsgSeqEscortCheck(); break;
		case 21: MsgSeqGameManiac(); break;
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
		printf("[ %zu/%zu ]: ", batch.index, batch.count);
		printf("'%.*s'\n", (int)cm.desc.count, cm.desc.items);
	}
	mg_ws_send(debug_conns[cm.conn_index], cm.msg.items, cm.msg.count, WEBSOCKET_OP_BINARY);
	BWriterFree(cm.msg);
	batch.index++;
}

void DebugEventHandler(struct mg_connection* c, int ev, void* ev_data) {
	(void)ev_data;
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
