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
	bool ready;
} GamePlayer;

typedef struct GamePlayers {
	GamePlayer* items;
	size_t count;
	size_t capacity;
} GamePlayers; // nob.h dynamic array

typedef struct Game {
	int users_count;
	GamePlayers players;
	bool debug;
	GameState state;
} Game;

extern Game game;

void GameUsersUpdate(struct mg_mgr* mgr);
void GameUserAdd(struct mg_connection* c);
void GameUserRemove(struct mg_connection* c);
void GamePlayerRemove(struct mg_connection* c);

bool HandleClientLobbyJoin(struct mg_connection* c, ByteReader* br);
bool HandleClientLobbyReady(struct mg_connection* c, ByteReader* br);
bool HandleClientLobbyLeave(struct mg_connection* c, ByteReader* br);

#endif /* GAME_LOGIC_H */

#ifdef GAME_LOGIC_IMPLEMENTATION

Game game;

void GameSend(struct mg_connection* c, ByteWriter* bw) {
	mg_ws_send(c, bw->sb.items, bw->sb.count, WEBSOCKET_OP_BINARY);
}

void GameSendAll(struct mg_mgr* mgr, ByteWriter* bw) {
	for (struct mg_connection* c = mgr->conns; c != NULL; c = c->next) {
		if (c->is_websocket && !c->is_client) GameSend(c, bw);
	}
}

void GameSendError(struct mg_connection* c, GameErrorType et) {
	ByteWriter bw = {0};
	ByteWriterU8(&bw, (uint8_t)GSMT_ERROR);
	ByteWriterU8(&bw, (uint8_t)et);
	GameSend(c, &bw);
	ByteWriterFree(bw);
}

void GameSendConfirm(struct mg_connection* c, GameConfirmType ct) {
	ByteWriter bw = {0};
	ByteWriterU8(&bw, (uint8_t)GSMT_CONFIRM);
	ByteWriterU8(&bw, (uint8_t)ct);
	GameSend(c, &bw);
	ByteWriterFree(bw);
}

void GameUsersUpdate(struct mg_mgr* mgr) {
	uint32_t viewers_count = game.users_count > game.players.count ? game.users_count - game.players.count : 0;
	MG_INFO(("viewers: %d\n", viewers_count));
	Nob_String_Builder player_names = {0};
	Nob_String_Builder player_states = {0};
	nob_da_foreach(GamePlayer, player, &game.players) {
		nob_sb_append_buf(&player_names, player->username.items, player->username.count);
		nob_da_append(&player_names, '\0');
		nob_da_append(&player_states, player->ready ? '1' : '0');
	}
	ByteWriter bw = ByteWriterBuild(NULL,
		BU8, GSMT_INFO_USERS,
		BU32, viewers_count,
		BU32, game.players.count,
		BSN, player_names.count, player_names.items,
		BSN, player_states.count, player_states.items);
	GameSendAll(mgr, &bw);
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
	game.users_count--;
	GamePlayerRemove(c);
	MG_INFO(("users: %d\n", game.users_count));
}

void GamePlayerRemove(struct mg_connection* c) {
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (p->c == c) {
			nob_sb_free(p->username);
			nob_da_remove_unordered(&game.players, p - game.players.items);
		}
	}
	GameUsersUpdate(c->mgr);
}

void GameSendState(struct mg_connection* c) {
	ByteWriter bw = {0};
	ByteWriterU8(&bw, (uint8_t)GSMT_GAME_STATE);
	ByteWriterU8(&bw, (uint8_t)game.state);
	GameSendAll(c->mgr, &bw);
	ByteWriterFree(bw);
}

void GameStart(struct mg_connection* c) {
	game.state = GS_DAY;
	GameSendState(c);
}

// --- HANDLERS ---

bool HandleClientConnect(struct mg_connection* c) {
	GameSendState(c);
}

bool HandleClientLobbyJoin(struct mg_connection* c, ByteReader* br) {
	bool result = true;
	uint32_t n;
	Nob_String_Builder username = {0};
	if (!ByteReaderU32(br, &n)) { nob_return_defer(false); }
	username = ByteReaderSBAlloc(br, n);
	nob_da_foreach(GamePlayer, p, &game.players) {
		// TODO: mg_strcmp
		if (p->username.count == username.count && strncmp(p->username.items, username.items, username.count) == 0) {
			if (game.debug) {
				p->c = c;
				GameUsersUpdate(c->mgr);
				nob_return_defer(true);
			}
			nob_return_defer(false);
		}
		if (p->c == c) {
			nob_return_defer(false);
		}
	}
	if (game.state == GS_DAY) {
		GameSendError(c, GE_JOIN_GAME_IN_PROGRESS);
		nob_return_defer(false);
	}
	GamePlayer player = {0};
	player.c = c;
	player.username = username; // TODO: check username
	nob_da_append(&game.players, player);
	GameUsersUpdate(c->mgr);
defer:
	if (result) { 
		GameSendConfirm(c, GC_JOIN_SUCCESS);
	} else {
		nob_sb_free(username);
	}
	return result;
}

bool HandleClientLobbyReady(struct mg_connection* c, ByteReader* br) {
	bool result = true;
	uint8_t ready;
	if (!ByteReaderU8(br, &ready))
		nob_return_defer(false);
	size_t ready_count = 0;
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (p->c == c) { p->ready = ready; }
		if (p->ready) { ready_count++; }
	}
	if (ready_count == game.players.count) {
		GameStart(c);
	}
	GameUsersUpdate(c->mgr);
defer:
	return result;
}

bool HandleClientLobbyLeave(struct mg_connection* c, ByteReader* br) {
	printf("GS_LOBBY: %d\n", GS_LOBBY);
	printf("game.state: %d\n", game.state);
	if (game.state != GS_LOBBY) {
		return false;
	}
	printf("!!!\n");
	GamePlayerRemove(c);
	GameSendConfirm(c, GC_LEAVE_SUCCESS);
	return true;
}

#endif /* GAME_LOGIC_IMPLEMENTATION */
