#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "nob.h"
#include "mongoose.h"
#include "game_enums.h"
#include "binary_rw.h"

typedef struct GameUser {
	struct mg_connection* c;
} GameUser;

typedef struct GamePlayer {
	struct mg_connection* c;
	Nob_String_Builder username;
	bool ready;
	bool ready_next;
	bool is_dead;
	GameRoleType role;
} GamePlayer;

typedef struct GamePlayers {
	GamePlayer* items;
	size_t count;
	size_t capacity;
} GamePlayers; // nob.h dynamic array

typedef struct Game {
	size_t users_count;
	GamePlayers players;
	size_t ready_next_count;
	bool debug;
	bool manual_roles;
	GameState state;
	BWriter history;
	int mafia_chose;
} Game;

extern Game game;

GamePlayer* GameGetPlayer(struct mg_connection* c);
void GameUsersUpdate(struct mg_mgr* mgr);
void GameUserAdd(struct mg_connection* c);
void GameUserRemove(struct mg_connection* c);
void GamePlayerRemove(struct mg_connection* c);

void HandleClientConnect(struct mg_connection* c);
bool HandleClientLobbyJoin(struct mg_connection* c, BReader* br);
bool HandleClientLobbyReady(struct mg_connection* c, BReader* br);
bool HandleClientReadyNext(struct mg_connection* c, BReader* br);
bool HandleClientLobbyLeave(struct mg_connection* c, BReader* br);
void HandleClientLobbyPoll(struct mg_connection* c, BReader* br);
void HandleClientDisconnect(struct mg_connection* c);

void GameTestSetRoles();

#endif /* GAME_LOGIC_H */

#ifdef GAME_LOGIC_IMPLEMENTATION

Game game;

GamePlayer* GameGetPlayer(struct mg_connection* c) {
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (c == p->c) { return p; }
	}
	return NULL;
}

int GameGetPlayerIndex(struct mg_connection* c) {
	int i = 0;
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (c == p->c) { return i; }
		i++;
	}
	return -1;
}

void GameSend(struct mg_connection* c, Nob_String_View sv) {
	if (!c) { return; }
	mg_ws_send(c, sv.data, sv.count, WEBSOCKET_OP_BINARY);
}

void GameSendAll(struct mg_mgr* mgr, Nob_String_View sv) {
	for (struct mg_connection* c = mgr->conns; c != NULL; c = c->next) {
		if (c->is_websocket && !c->is_client) { GameSend(c, sv); }
	}
}

void GameSendError(struct mg_connection* c, GameErrorType et) {
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_ERROR, BU8, et);
	GameSend(c, nob_sb_to_sv(bw_temp));
}

void GameSendConfirm(struct mg_connection* c, GameConfirmType ct) {
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_CONFIRM, BU8, ct);
	GameSend(c, nob_sb_to_sv(bw_temp));
}

void GameUpdateReadyNext(struct mg_mgr* mgr) {
	bw_temp.count = 0;
	BWriterAppend(&bw_temp,
		BU8, GSMT_INFO_READY_NEXT,
		BU8, game.ready_next_count == game.players.count,
		BU32, game.ready_next_count);
	GameSendAll(mgr, nob_sb_to_sv(bw_temp));
}

void GameUsersUpdate(struct mg_mgr* mgr) {
	uint32_t viewers_count = game.users_count > game.players.count ? game.users_count - game.players.count : 0;
	Nob_String_Builder player_names = {0};
	Nob_String_Builder player_states = {0};
	Nob_String_Builder player_dead = {0};
	nob_da_foreach(GamePlayer, player, &game.players) {
		bw_temp.count = 0;
		BWriterAppend(&bw_temp, BU8, GSMT_INFO_PLAYER, BU8, player->ready_next);
		GameSend(player->c, nob_sb_to_sv(bw_temp));
		nob_sb_append_buf(&player_names, player->username.items, player->username.count);
		nob_da_append(&player_names, '\0');
		nob_da_append(&player_states, player->ready ? '1' : '0');
		nob_da_append(&player_dead, player->is_dead ? '1' : '0');
	}
	bw_temp.count = 0;
	BWriterAppend(&bw_temp,
		BU8, GSMT_INFO_USERS,
		BU32, viewers_count,
		BU32, game.players.count,
		BSN, player_names.count, player_names.items,
		BSN, player_states.count, player_states.items,
		BSN, player_dead.count, player_dead.items);
	GameSendAll(mgr, nob_sb_to_sv(bw_temp));
	nob_sb_free(player_names);
	GameUpdateReadyNext(mgr);
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
	for (size_t i = 0; i < game.players.count; i++) {
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
		if (player_index == (unsigned int)-1 || player_index == curr_player_index) {
			GameSend(c, nob_sb_to_sv(bw));
		}
		br.i += msg_count;
	}
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_HISTORY_END);
	GameSend(c, nob_sb_to_sv(bw_temp));
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

void GameSetRolesCount(int* count_lookup, int n) {
	int mafia_count = n / 3;
	//int town_count = n - mafia_count;
	count_lookup[GRT_MAFIA]    = mafia_count;
	count_lookup[GRT_SERIF]    = 1;
	count_lookup[GRT_DOCTOR]   = 1;
	count_lookup[GRT_ESCORT]   = 1;
	count_lookup[GRT_MANIAC]   = n > 7 ? 1 : 0;
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
	game.state = GS_FIRST_DAY;
	if (game.players.count < 5) { return; }
	if (!game.manual_roles) {
		// Give Roles
		int count_lookup[GRT_LAST_];
		GameSetRolesCount(count_lookup, game.players.count);
		int role = GRT_VILLAGER;
		nob_da_foreach(GamePlayer, p, &game.players) {
			if (count_lookup[role] == 0) { role++; }
			if (role == GRT_LAST_) { NOB_UNREACHABLE("count_lookup ended"); }
			p->role = role;
			count_lookup[role]--;
		}
		// Shuffle
		for (size_t i = 0; i < game.players.count; i++) {
			int j = rand() % GAT_LAST_;
			GameRoleType* i_role = &game.players.items[i].role;
			GameRoleType* j_role = &game.players.items[j].role;
			GameRoleType tmp = *i_role;
			*i_role = *j_role;
			*j_role = tmp;
		}
	}
	//for (int i = 0; i < game.players.count; i++) {
	//	printf("Player %d: %s\n", i, GRT_NAMES[game.players.items[i].role]);
	//}
	// Send
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_CLEAR);
	GameSendAction(c, nob_sb_to_sv(bw_temp), -1);
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_STARTED);
	GameSendAction(c, nob_sb_to_sv(bw_temp), -1);
	int i = 0;
	nob_da_foreach(GamePlayer, p, &game.players) {
		bw_temp.count = 0;
		p->ready = 0;
		BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_ROLE, BU8, p->role);
		GameSendAction(c, nob_sb_to_sv(bw_temp), i);
		i++;
	}
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_DAY_ENDED);
	GameSendAction(c, nob_sb_to_sv(bw_temp), -1);
	GameUsersUpdate(c->mgr);
}

// TODO: proper prefix for this funcs
void GameNight(struct mg_connection* c) {
	game.state = GS_NIGHT;
	game.mafia_chose = -1;
	nob_da_foreach(GamePlayer, p, &game.players) { p->ready_next = 0; }
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_NIGHT_STARTED);
	GameSendAction(c, nob_sb_to_sv(bw_temp), -1);
	for (size_t i = 0; i < game.players.count; i++) {
		GamePlayer p = game.players.items[i];
		bw_temp.count = 0;
		BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_NIGHT_ROLE_ACTION, BU8, p.role);
		GameSendAction(c, nob_sb_to_sv(bw_temp), i);
		switch (p.role) {
			case GRT_MAFIA:
				bw_temp.count = 0;
				BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_POLL);
				GameSendAction(c, nob_sb_to_sv(bw_temp), i);
				break;
			default:
				break;
		}
	}
	GameUpdateReadyNext(c->mgr);
}

void GameDay(struct mg_connection* c) {
	game.state = GS_DAY;
	if (game.mafia_chose == -1) {
		for (size_t i = 0; i < game.players.count; i++) {
			GamePlayer* p = &game.players.items[i];
			if (!p->is_dead && p->role != GRT_MAFIA) {
				game.mafia_chose = i;
			}
		}
		if (game.mafia_chose == -1) { NOB_UNREACHABLE("mafia autokill"); }
	}
	// MESSAGE
	bw_temp.count = 0;
	BWriterAppend(&bw_temp, BU8, GSMT_GAME_ACTION, BU8, GAT_DAY_STARTED);
	GameSendAction(c, nob_sb_to_sv(bw_temp), -1);
	// MAFIA KILL
	GamePlayer* player_killed = &game.players.items[game.mafia_chose];
	player_killed->is_dead = true;
	bw_temp.count = 0;
	BWriterAppend(&bw_temp,
			BU8, GSMT_GAME_ACTION,
			BU8, GAT_PLAYER_KILLED,
			BU32, game.mafia_chose);
	GameSendAction(c, nob_sb_to_sv(bw_temp), -1);

	GameUsersUpdate(c->mgr);
}


// --- HANDLERS ---

void HandleClientConnect(struct mg_connection* c) {
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
		if (nob_sv_eq(nob_sb_to_sv(p->username), nob_sb_to_sv(username))) {
			if (game.debug || p->c == NULL) {
				p->c = c;
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
defer:
	if (result) {
		GameSendConfirm(c, GC_JOIN_SUCCESS);
		GameSendHistory(c);
	} else {
		nob_sb_free(username);
	}
	GameUsersUpdate(c->mgr);
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
	(void)br;
	bool result = true;
	game.ready_next_count = 0;
	nob_da_foreach(GamePlayer, p, &game.players) {
		if (p->c == c) { p->ready_next = !p->ready_next; }
		if (p->ready_next) { game.ready_next_count++; }
	}
	if (game.ready_next_count == game.players.count) {
		nob_da_foreach(GamePlayer, p, &game.players) {
			p->ready_next = 0;
		}
		game.ready_next_count = 0;
		switch (game.state) {
			case GS_FIRST_DAY:
			case GS_DAY:
				GameNight(c);
				break;
			case GS_NIGHT:
				GameDay(c);
				break;
			default:
				break;
		}
	}
	GameUpdateReadyNext(c->mgr);
	GameUsersUpdate(c->mgr);
	return result;
}

bool HandleClientLobbyLeave(struct mg_connection* c, BReader* br) {
	(void)br;
	if (game.state != GS_LOBBY) {
		GameSendError(c, GE_LEAVE_GAME_IN_PROGRESS);
		return false;
	}
	GamePlayerRemove(c);
	GameSendConfirm(c, GC_LEAVE_SUCCESS);
	return true;
}

void HandleClientLobbyPoll(struct mg_connection* c, BReader* br) {
	uint32_t index;
	if (!BReadU32(br, &index)) { return; }
	if (index >= game.players.count) { return; }
	int voter_index = GameGetPlayerIndex(c);
	if (voter_index == -1) { return; }
	GamePlayer* p = &game.players.items[voter_index];
	if (p->is_dead) { return; }
	if (game.state == GS_NIGHT) {
		switch (p->role) {
			case GRT_MAFIA:
				game.mafia_chose = index;
				bw_temp.count = 0;
				BWriterAppend(&bw_temp,
					BU8, GSMT_GAME_ACTION,
					BU8, GAT_POLL_MAFIA_CHOSE,
					BU32, voter_index,
					BU32, game.mafia_chose);
				int i = 0;
				nob_da_foreach(GamePlayer, p, &game.players) {
					if (p->role == GRT_MAFIA) { GameSendAction(c, nob_sb_to_sv(bw_temp), i); }
					i++;
				}
				break;
			default:
				break;
		}
	}
	GameUsersUpdate(c->mgr);
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
