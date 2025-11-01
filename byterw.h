// Simple I/O Streams for bytes

#ifndef BYTERW_H
#define BYTERW_H

#include "nob.h"

typedef struct ByteReader {
	Nob_String_View sv;
	int i;
} ByteReader;

bool ByteReaderU8 (ByteReader* br, uint8_t*  out);
bool ByteReaderU16(ByteReader* br, uint16_t* out);
bool ByteReaderU32(ByteReader* br, uint32_t* out);
bool ByteReaderU64(ByteReader* br, uint64_t* out);
bool ByteReaderN  (ByteReader* br, uint8_t*  out, size_t n);
Nob_String_Builder ByteReaderSBAlloc(ByteReader* br, size_t n);

typedef struct ByteWriter {
	Nob_String_Builder sb;
} ByteWriter;

void ByteWriterU8 (ByteWriter* bw, const uint8_t  in);
void ByteWriterU16(ByteWriter* bw, const uint16_t in);
void ByteWriterU32(ByteWriter* bw, const uint32_t in);
void ByteWriterU64(ByteWriter* bw, const uint64_t in);
void ByteWriterN  (ByteWriter* bw, const uint8_t* in, size_t n);
void ByteWriterFree(ByteWriter bw);

#ifdef BYTERW_IMPLEMENTATION

#define BR_READ(type_, amount_) \
	do { \
		if (br->i + amount_ > br->sv.count) \
			return false; \
		memcpy(out, &br->sv.data[br->i], amount_); \
		br->i += amount_; \
		return true; \
	} while(0);

bool ByteReaderU8 (ByteReader* br, uint8_t*  out) { BR_READ(uint8_t,  1); }
bool ByteReaderU16(ByteReader* br, uint16_t* out) { BR_READ(uint16_t, 2); }
bool ByteReaderU32(ByteReader* br, uint32_t* out) { BR_READ(uint32_t, 4); }
bool ByteReaderU64(ByteReader* br, uint64_t* out) { BR_READ(uint64_t, 8); }
bool ByteReaderN  (ByteReader* br, uint8_t*  out, size_t n) { BR_READ(uint8_t,  n); }
Nob_String_Builder ByteReaderSBAlloc(ByteReader* br, size_t n) {
	Nob_String_Builder sb = {0};
	nob_da_reserve(&sb, n);
	sb.count = n;
	if (!ByteReaderN(br, sb.items, sb.count))
		return (Nob_String_Builder){ 0 };
	return sb;
}

#define BR_WRITE_SCALAR(amount_) \
	nob_sb_append_buf(&bw->sb, (uint8_t*)&in, amount_);

#define BR_WRITE(amount_) \
	nob_sb_append_buf(&bw->sb, (uint8_t*)in, amount_);

void ByteWriterU8 (ByteWriter* bw, const uint8_t  in) { BR_WRITE_SCALAR(1); }
void ByteWriterU16(ByteWriter* bw, const uint16_t in) { BR_WRITE_SCALAR(2); }
void ByteWriterU32(ByteWriter* bw, const uint32_t in) { BR_WRITE_SCALAR(4); }
void ByteWriterU64(ByteWriter* bw, const uint64_t in) { BR_WRITE_SCALAR(8); }
void ByteWriterN  (ByteWriter* bw, const uint8_t* in, size_t n) { BR_WRITE(n); }
void ByteWriterFree(ByteWriter bw) { nob_sb_free(bw.sb); };

#endif /* BYTERW_IMPLEMENTATION */
#endif /* BYTERW_H */
