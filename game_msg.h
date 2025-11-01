#ifndef GAME_MSG_H
#define GAME_MSG_H

#include "nob.h"

// --- Game Server Message Types (GSMT) ---

#define GSMT \
	X(GSMT_INFO_VIEWERS) \
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

#ifdef GAME_MSG_IMPLEMENTATION

#define X(name_) #name_,
const char *gcmt_names[] = { GCMT };
#undef X

#define X(name_) #name_,
const char *gcmt_names[] = { GCMT };
#undef X

extern Nob_String_Builder gmt_js;

void GameMessageTypesGenerateJS() {
	nob_sb_append_cstr(&gmt_js, "const GSMT = {\n");
	for (int i = 0; i < GSMT_LAST_; i++) {
		nob_sb_appendf(&gmt_js, "\t%s: %d,\n", gsmt_names[i]+strlen("GSMT_"), i);
	}
	nob_sb_append_cstr(&gmt_js, "};\n");
	nob_sb_append_cstr(&gmt_js, "const GCMT = {\n");
	for (int i = 0; i < GCMT_LAST_; i++) {
		nob_sb_appendf(&gmt_js, "\t%s: %d,\n", gcmt_names[i]+strlen("GCMT_"), i);
	}
	nob_sb_append_cstr(&gmt_js, "};");
	nob_sb_append_null(&gmt_js);
}

#endif /* GAME_MSG_IMPLEMENTATION */
#endif /* GAME_MSG_H */
