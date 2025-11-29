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
	bool ready_next;
	GameRoleType role;
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
	BWriter history;
} Game;

extern Game game;

void GameUsersUpdate(struct mg_mgr* mgr);
void GameUserAdd(struct mg_connection* c);
void GameUserRemove(struct mg_connection* c);
void GamePlayerRemove(struct mg_connection* c);

bool HandleClientConnect(struct mg_connection* c);
bool HandleClientLobbyJoin(struct mg_connection* c, BReader* br);
bool HandleClientLobbyReady(struct mg_connection* c, BReader* br);
bool HandleClientLobbyLeave(struct mg_connection* c, BReader* br);
void HandleClientDisconnect(struct mg_connection* c);
bool HandleClientReadyNext(struct mg_connection* c, BReader* br);

void GameTestSetRoles();

#endif /* GAME_LOGIC_H */

#ifdef GAME_LOGIC_IMPLEMENTATION

Game game;

void GameSend(struct mg_connection* c, Nob_String_View sv) {
	mg_ws_send(c, sv.data, sv.count, WEBSOCKET_OP_BINARY);
}

void GameSendAll(struct mg_mgr* mgr, Nob_String_View sv) {
	for (struct mg_connection* c = mgr->conns; c != NULL; c = c->next) {
		if (c->is_websocket && !c->is_client) { GameSend(c, sv); }
	}
}

void GameSendError(struct mg_connection* c, GameErrorType et) {
	BWriter bw = {0};
	BWriteU8(&bw, (uint8_t)GSMT_ERROR);
	BWriteU8(&bw, (uint8_t)et);
	GameSend(c, nob_sb_to_sv(bw));
	BWriterFree(bw);
}

void GameSendConfirm(struct mg_connection* c, GameConfirmType ct) {
	BWriter bw = {0};
	BWriteU8(&bw, (uint8_t)GSMT_CONFIRM);
	BWriteU8(&bw, (uint8_t)ct);
	GameSend(c, nob_sb_to_sv(bw));
	BWriterFree(bw);
}

void GameUsersUpdate(struct mg_mgr* mgr) {
	uint32_t viewers_count = game.users_count > game.players.count ? game.users_count - game.players.count : 0;
	Nob_String_Builder player_names = {0};
	Nob_String_Builder player_states = {0};
	nob_da_foreach(GamePlayer, player, &game.players) {
		nob_sb_append_buf(&player_names, player->username.items, player->username.count);
		nob_da_append(&player_names, '\0');
		nob_da_append(&player_states, player->ready ? '1' : '0');
	}
	BWriter bw = BWriterBuild(NULL,
		BU8, GSMT_INFO_USERS,
		BU32, viewers_count,
		BU32, game.players.count,
		BSN, player_names.count, player_names.items,
		BSN, player_states.count, player_states.items);
	GameSendAll(mgr, nob_sb_to_sv(bw));
	BWriterFree(bw);
	nob_sb_free(player_names);
}

void GameUserAdd(struct mg_connection* c) {
	GameUser* user = calloc(1, sizeof(*user));
	assert(user != NULL);
	user->c = c;
	game.users_count++;
	GameUsersUpdate(c->mgr);
}

void GameUserRemove(struct mg_connection* c) {
	game.users_count--;
	GamePlayerRemove(c);
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

void GameSendHistory(struct mg_connection* c) {
	BReader br = {
		.data = game.history.items,
		.count = game.history.count,
		.i = 0
	};

	size_t curr_player_index = -1;
	for (int i = 0; i < game.players.count; i++) {
		if (game.players.items[i].c == c) {
			curr_player_index = i;
		}
	}
	while (br.i < br.count) {
		uint32_t player_index;
		if (!BReadU32(&br, &player_index)) { return; }
		uint32_t msg_count;
		if (!BReadU32(&br, &msg_count)) { return; }
		BWriter bw = {
			.items = (char*)(br.data + br.i),
			.count = msg_count
		};
		if (player_index == -1 || player_index == curr_player_index) {
			GameSend(c, nob_sb_to_sv(bw));
		}
		br.i += msg_count;
	}
}

void GameSendAction(struct mg_connection* c, Nob_String_View sv, int player_index) {
	BWriteU32(&game.history, player_index);
	if (player_index == -1) {
		BWriteSN(&game.history, sv.data, sv.count);
		GameSendAll(c->mgr, sv);
		return;
	}
	BWriteSN(&game.history, sv.data, sv.count);
	GameSend(game.players.items[player_index].c, sv);
}

int GameSetRolesCount(int* count_lookup, int n) {
	int mafia_count = n / 3;
	int town_count = n - mafia_count;
	count_lookup[GRT_MAFIA]    = mafia_count;
	count_lookup[GRT_SERIF]    = 1 + town_count / 6;
	count_lookup[GRT_DOCTOR]   = 1 + town_count / 6;
	count_lookup[GRT_ESCORT]   = town_count / 7;
	count_lookup[GRT_MANIAC]   = town_count / 7;
	count_lookup[GRT_VILLAGER] = n;
	for (int i = 1; i < GRT_LAST_; i++) {
		count_lookup[GRT_VILLAGER] -= count_lookup[i];
	}
	printf("Players: %d\n", n);
	for (int i = 0; i < GRT_LAST_; i++) {
		printf("  %s: %d\n", GRT_NAMES[i], count_lookup[i]);
	}
}

void GameTestSetRoles() {
	int count_lookup[GRT_LAST_];
	for (int i = 5; i < 100; i++) {
		GameSetRolesCount(count_lookup, i);
	}
}

void GameStart(struct mg_connection* c) {
	if (game.players.count < 5) { return; }
	// Give Roles
	int count_lookup[GRT_LAST_];
	GameSetRolesCount(count_lookup, game.players.count);
	int role = 0;
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (count_lookup[role] == 0) { role++; }
		if (role == GRT_LAST_) { NOB_UNREACHABLE("count_lookup ended"); }
		p->role = role;
		count_lookup[role]--;
	}
	// Shuffle
	for (int i = 0; i < game.players.count; i++) {
		int j = rand() % GAT_LAST_;
		GameRoleType* i_role = &game.players.items[i].role;
		GameRoleType* j_role = &game.players.items[j].role;
		GameRoleType tmp = *i_role;
		*i_role = *j_role;
		*j_role = tmp;
	}
	//for (int i = 0; i < game.players.count; i++) {
	//	printf("Player %d: %s\n", i, GRT_NAMES[game.players.items[i].role]);
	//}
	// Send
	game.state = GS_FIRST_DAY;
	{
		BWriter bw = {0};
		BWriteU8(&bw, (uint8_t)GSMT_GAME_ACTION);
		BWriteU8(&bw, (uint8_t)GAT_CLEAR);
		GameSendAction(c, nob_sb_to_sv(bw), -1);
		bw.count = 0;
		BWriteU8(&bw, (uint8_t)GSMT_GAME_ACTION);
		BWriteU8(&bw, (uint8_t)GAT_STARTED);
		GameSendAction(c, nob_sb_to_sv(bw), -1);
		int i = 0;
		nob_da_foreach(GamePlayer, p, &game.players) {
			bw.count = 0;
			p->ready = 0;
			BWriterBuild(&bw, BU8, GSMT_GAME_ACTION, BU8, GAT_ROLE, BU8, p->role);
			if (p->c == NULL) { continue; }
			GameSendAction(c, nob_sb_to_sv(bw), i);
			i++;
		}
		bw.count = 0;
		BWriterBuild(&bw, BU8, GSMT_GAME_ACTION, BU8, GAT_DAY_ENDED);
		GameSendAction(c, nob_sb_to_sv(bw), -1);
		GameUsersUpdate(c->mgr);
		BWriterFree(bw);
	}
}

// --- HANDLERS ---

bool HandleClientConnect(struct mg_connection* c) {
	GameSendHistory(c);
}

bool HandleClientLobbyJoin(struct mg_connection* c, BReader* br) {
	bool result = true;
	uint32_t n;
	Nob_String_Builder username = {0};
	if (!BReadU32(br, &n)) { nob_return_defer(false); }
	if (n > 32) {
		GameSendError(c, GE_JOIN_NAME_TOO_LONG);
		nob_return_defer(false);
	}
	username = BReadSB(br, n);
	nob_da_foreach(GamePlayer, p, &game.players) {
		// TODO: mg_strcmp
		if (p->username.count == username.count && strncmp(p->username.items, username.items, username.count) == 0) {
			if (game.debug || p->c == NULL) {
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
	if (game.state != GS_LOBBY) {
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
		GameSendHistory(c);
		GameSendConfirm(c, GC_JOIN_SUCCESS);
	} else {
		nob_sb_free(username);
	}
	return result;
}

bool HandleClientLobbyReady(struct mg_connection* c, BReader* br) {
	bool result = true;
	uint8_t ready;
	if (!BReadU8(br, &ready))
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

bool HandleClientReadyNext(struct mg_connection* c, BReader* br) {
	bool result = true;
	uint8_t ready;
	if (!BReadU8(br, &ready))
		nob_return_defer(false);
	size_t ready_count = 0;
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (p->c == c) { p->ready_next = ready; }
		if (p->ready_next) { ready_count++; }
	}
	if (ready_count == game.players.count) {
		BWriter bw = {0};
		BWriterBuild(&bw, BU8, GSMT_GAME_ACTION, BU8, GAT_NIGHT_STARTED);
		GameSendAction(c, nob_sb_to_sv(bw), -1);
		BWriterFree(bw);
	}
	GameUsersUpdate(c->mgr);
defer:
	return result;
}

bool HandleClientLobbyLeave(struct mg_connection* c, BReader* br) {
	if (game.state != GS_LOBBY) {
		return false;
	}
	GamePlayerRemove(c);
	GameSendConfirm(c, GC_LEAVE_SUCCESS);
	return true;
}

void HandleClientDisconnect(struct mg_connection* c) {
	if (game.state == GS_LOBBY) {
		GameUserRemove(c);
		return;
	}
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (p->c == c) { p->c = NULL; }
	}
}

#endif /* GAME_LOGIC_IMPLEMENTATION */
