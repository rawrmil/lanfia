#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "nob.h"
#include "mongoose.h"
#include "game_enums.h"

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

typedef struct Game {
	int users_count;
	GamePlayers players;
} Game;

extern Game game;

void GameUsersUpdate(struct mg_mgr* mgr);
void GameUserAdd(struct mg_connection* c);
void GameUserRemove(struct mg_connection* c);

bool HandleClientLobbyJoin(struct mg_connection* c, ByteReader* br);

#endif /* GAME_LOGIC_H */

#ifdef GAME_LOGIC_IMPLEMENTATION

Game game;

void GameUsersUpdate(struct mg_mgr* mgr) {
	// [ msg_type | viewers | players.count | ( names_size | names_array ) ]
	uint32_t viewers_count = game.users_count > game.players.count ? game.users_count-game.players.count : 0;
	MG_INFO(("viewers: %d\n", viewers_count));
	ByteWriter bw = {0};
	ByteWriterU8(&bw, GSMT_INFO_USERS);
	ByteWriterU32(&bw, viewers_count);
	ByteWriterU32(&bw, (uint32_t)game.players.count);
	Nob_String_Builder player_names = {0};
	nob_da_foreach(GamePlayer, player, &game.players) {
		nob_sb_append_buf(&player_names, player->username.items, player->username.count);
		nob_da_append(&player_names, '\0');
	}
	ByteWriterSN(&bw, player_names.items, player_names.count);
	for (struct mg_connection* c = mgr->conns; c != NULL; c = c->next) {
		if (c->is_websocket)
			mg_ws_send(c, bw.sb.items, bw.sb.count, WEBSOCKET_OP_BINARY);
	}
	ByteWriterFree(bw);
	nob_sb_free(player_names);
}

void GameUserAdd(struct mg_connection* c) {
	GameUser* user = calloc(1, sizeof(*user));
	assert(user != NULL);
	user->c = c;
	game.users_count++;
	GameUsersUpdate(c->mgr);
	MG_INFO(("users: %d\n", game.users_count));
}

void GameUserRemove(struct mg_connection* c) {
	// TODO: IF PHASE == LOBBY
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (p->c == c) {
			nob_sb_free(p->username);
			nob_da_remove_unordered(&game.players, p - game.players.items);
		}
	}
	game.users_count--;
	GameUsersUpdate(c->mgr);
	MG_INFO(("users: %d\n", game.users_count));
}

// --- HANDLERS ---

bool HandleClientLobbyJoin(struct mg_connection* c, ByteReader* br) {
	bool result = true;
	uint32_t n;
	if (!ByteReaderU32(br, &n))
		nob_return_defer(false);
	Nob_String_Builder username = ByteReaderSBAlloc(br, n);
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (p->c == c) {
			// TODO: already connected
			nob_return_defer(false);
		}
	}
	GamePlayer player = {0};
	player.c = c;
	player.username = username; // TODO: check username
	nob_da_append(&game.players, player);
	GameUsersUpdate(c->mgr);
defer:
	if (result == false) nob_sb_free(username);
	return result;
}

#endif /* GAME_LOGIC_IMPLEMENTATION */
