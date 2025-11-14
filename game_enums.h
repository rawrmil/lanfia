#ifndef GAME_ENUMS_H
#define GAME_ENUMS_H

#include "nob.h"
#include "game_logic.h"

// --- Game Server Message Types (GSMT) ---

#define GSMT \
	X(GSMT_NO_MSG) \
	X(GSMT_INFO_USERS) \
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
	X(GCMT_LAST_) \

#define X(name_) name_,
enum { GCMT };
#undef X

// --- Game State (GS) ---

#define GS \
	X(GS_LOBBY) \
	X(GS_ROLES) \
	X(GS_DAY) \
	X(GS_NIGHT) \
	X(GS_RESULTS) \
	X(GS_LAST_)

#define X(name_) name_,
enum { GS };
#undef X

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
	nob_sb_appendf(&gmt_js, "const debug = %s;\n", game.debug ? "true" : "false");
	nob_sb_append_null(&gmt_js);
}

#endif /* GAME_ENUMS_IMPLEMENTATION */
