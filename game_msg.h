#ifndef GAME_MSG_H
#define GAME_MSG_H

#include "nob.h"

// --- Game Server Message Types (GSMT) ---

#define GSMT \
	X(GSMT_INFO_USERS) \
	X(GSMT_LAST_)\

#define X(name_) name_,
enum GameServerMessageTypes { GSMT };
#undef X

// --- Game Client Message Types (GCMT) ---

#define GCMT \
	X(GCMT_LOBBY_JOIN) \
	X(GCMT_LAST_)\

#define X(name_) name_,
enum GameClientMessageTypes { GCMT };
#undef X

// --- JS ---

extern Nob_String_Builder gmt_js;

void GameMessageTypesGenerateJS();

#endif /* GAME_MSG_H */

#ifdef GAME_MSG_IMPLEMENTATION

#define X(name_) #name_,
const char *gsmt_names[] = { GSMT };
#undef X

#define X(name_) #name_,
const char *gcmt_names[] = { GCMT };
#undef X

Nob_String_Builder gmt_js;

#define JS_ENUM_GENERATE(name_) \
	do { \
		nob_sb_append_cstr(&gmt_js, "const "#name_" = {\n"); \
		for (int i = 0; i < name_##_LAST_; i++) { \
			nob_sb_appendf(&gmt_js, "\t%s: %d,\n", gsmt_names[i]+strlen(#name_"_"), i); \
		} \
		nob_sb_append_cstr(&gmt_js, "};\n"); \
	} while(0);

void GameMessageTypesGenerateJS() {
	JS_ENUM_GENERATE(GSMT);
	JS_ENUM_GENERATE(GCMT);
	nob_sb_append_null(&gmt_js);
}

#endif /* GAME_MSG_IMPLEMENTATION */
