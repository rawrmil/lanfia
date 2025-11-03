#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "nob.h"
#include "mongoose.h"
#include "game_msg.h"

typedef struct GameUser {
	struct mg_connection* c;
} GameUser;

typedef struct GamePlayer {
	struct mg_connection* c;
	Nob_String_Builder username;
} GamePlayer;

typedef struct GamePlayers {
	GamePlayer* items;
	size_t count;
	size_t capacity;
} GamePlayers; // nob.h dynamic array

extern int users_count;
extern GamePlayers players;

void GameUsersUpdate(struct mg_mgr* mgr);
void GameUserAdd(struct mg_connection* c);
void GameUserRemove(struct mg_connection* c);

bool HandleClientLobbyJoin(struct mg_connection* c, ByteReader* br);

#endif /* GAME_LOGIC_H */

#ifdef GAME_LOGIC_IMPLEMENTATION

int users_count = 0;
GamePlayers players = {0};

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
			nob_sb_free(p->username);
			nob_da_remove_unordered(&players, p - players.items);
		}
	}
	users_count--;
	GameUsersUpdate(c->mgr);
	MG_INFO(("users: %d\n", users_count));
}

// --- HANDLERS ---

bool HandleClientLobbyJoin(struct mg_connection* c, ByteReader* br) {
	bool result = true;
	uint32_t n;
	if (!ByteReaderU32(br, &n))
		nob_return_defer(false);
	Nob_String_Builder username = ByteReaderSBAlloc(br, n);
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
	if (result == false) nob_sb_free(username);
	return result;
}

#endif /* GAME_LOGIC_IMPLEMENTATION */
