#ifndef GAME_ENUMS_H
#define GAME_ENUMS_H

#include "nob.h"

// --- Game Server Message Types (GSMT) ---

#define GSMT \
	X(GSMT_NO_MSG) \
	X(GSMT_INFO_PLAYER) \
	X(GSMT_INFO_USERS) \
	X(GSMT_INFO_READY_NEXT) \
	X(GSMT_GAME_ACTION) \
	X(GSMT_ERROR) \
	X(GSMT_CONFIRM) \
	X(GSMT_HISTORY_END) \
	X(GSMT_TIMER) \
	X(GSMT_LAST_) \

#define X(name_) name_,
enum { GSMT };
#undef X

// --- Game Client Message Types (GCMT) ---

#define GCMT \
	X(GCMT_NO_MSG) \
	X(GCMT_LOBBY_JOIN) \
	X(GCMT_LOBBY_LEAVE) \
	X(GCMT_LOBBY_READY) \
	X(GCMT_READY_NEXT) \
	X(GCMT_POLL) \
	X(GCMT_DEBUG_SET_ROLE) \
	X(GCMT_LAST_) \

#define X(name_) name_,
enum { GCMT };
#undef X

// --- Game State (GS) ---

#define GS \
	X(GS_LOBBY) \
	X(GS_FIRST_DAY) \
	X(GS_DAY) \
	X(GS_NIGHT) \
	X(GS_LAST_)

#define X(name_) name_,
typedef enum GameState { GS } GameState;
#undef X

// --- Game Join Error ---

#define GE \
	X(GE_JOIN_GAME_IN_PROGRESS) \
	X(GE_LEAVE_GAME_IN_PROGRESS) \
	X(GE_JOIN_NAME_TOO_LONG) \
	X(GE_LAST_) \

#define X(name_) name_,
typedef enum GameErrorType { GE } GameErrorType;
#undef X

// --- Game Confirm ---

#define GC \
	X(GC_JOIN_SUCCESS) \
	X(GC_LEAVE_SUCCESS) \
	X(GC_LAST_) \

#define X(name_) name_,
typedef enum GameConfirmType { GC } GameConfirmType;
#undef X

// --- Game Action Message Types (GAT) ---

#define GAT \
	X(GAT_CLEAR) \
	X(GAT_STARTED) \
	X(GAT_ROLE) \
	X(GAT_DAY_STARTED) \
	X(GAT_DAY_ENDED) \
	X(GAT_NIGHT_STARTED) \
	X(GAT_NIGHT_ROLE_ACTION) \
	X(GAT_POLL) \
	X(GAT_POLL_CHOSE) \
	X(GAT_PLAYER_KILLED) \
	X(GAT_PLAYER_KICKED) \
	X(GAT_PLAYER_CHECKED) \
	X(GAT_PLAYER_HEALED) \
	X(GAT_MAFIA_WON) \
	X(GAT_TOWN_WON) \
	X(GAT_MANIAC_WON) \
	X(GAT_RESULTS) \
	X(GAT_LAST_) \

#define X(name_) name_,
typedef enum GameActionType { GAT } GameActionMsgType;
#undef X

// --- Game Role Type (GRT) ---

#define GRT \
	X(GRT_VILLAGER) \
	X(GRT_MAFIA) \
	X(GRT_SERIF) \
	X(GRT_DOCTOR) \
	X(GRT_ESCORT) \
	X(GRT_MANIAC) \
	X(GRT_LAST_) \

#define X(name_) name_,
typedef enum GameRoleType { GRT } GameRoleType;
#undef X

// --- Names ---
extern const char *GRT_NAMES[];

// --- JS ---

extern Nob_String_Builder gmt_js;

void GameMessageTypesGenerateJS();

#endif /* GAME_ENUMS_H */

#ifdef GAME_ENUMS_IMPLEMENTATION

#define X(name_) #name_,
const char *GSMT_NAMES[] = { GSMT };
#undef X

#define X(name_) #name_,
const char *GCMT_NAMES[] = { GCMT };
#undef X

#define X(name_) #name_,
const char *GS_NAMES[] = { GS };
#undef X

#define X(name_) #name_,
const char *GE_NAMES[] = { GE };
#undef X

#define X(name_) #name_,
const char *GC_NAMES[] = { GC };
#undef X

#define X(name_) #name_,
const char *GAT_NAMES[] = { GAT };
#undef X

#define X(name_) #name_,
const char *GRT_NAMES[] = { GRT };
#undef X

Nob_String_Builder gmt_js;

#define JS_ENUM_GENERATE(name_) \
	do { \
		nob_sb_append_cstr(&gmt_js, "const "#name_" = {\n"); \
		for (int i = 0; i < name_##_LAST_; i++) { \
			nob_sb_appendf(&gmt_js, "\t%s: %d,\n", name_##_NAMES[i]+strlen(#name_"_"), i); \
		} \
		nob_sb_append_cstr(&gmt_js, "};\n"); \
	} while(0);

void GameMessageTypesGenerateJS() {
	JS_ENUM_GENERATE(GSMT);
	JS_ENUM_GENERATE(GCMT);
	JS_ENUM_GENERATE(GS);
	JS_ENUM_GENERATE(GE);
	JS_ENUM_GENERATE(GC);
	JS_ENUM_GENERATE(GAT);
	JS_ENUM_GENERATE(GRT);
	nob_sb_appendf(&gmt_js, "const debug = %s;\n", game.debug ? "true" : "false"); // TODO: by flag
	nob_sb_append_null(&gmt_js);
}

#endif /* GAME_ENUMS_IMPLEMENTATION */
