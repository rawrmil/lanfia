// Simple I/O Streams for bytes

#ifndef BYTERW_H
#define BYTERW_H

#include "nob.h"

typedef struct ByteReader {
	Nob_String_View sv;
	int i;
} ByteReader;

enum {
	BNULL,
	BU8,
	BU16,
	BU32,
	BU64,
	BN,
	BSN,
};

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
void ByteWriterSN (ByteWriter* bw, const uint8_t* in, size_t n); // [ size(4) | data(N) ]
ByteWriter _ByteWriterBuild(ByteWriter bw, ...);
void ByteWriterFree(ByteWriter bw);

#endif /* BYTERW_H */

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
void ByteWriterSN (ByteWriter* bw, const uint8_t* in, size_t n) { // TODO: same with BR
	ByteWriterU32(bw, (uint32_t)n);
	ByteWriterN(bw, in, n);
}
ByteWriter _ByteWriterBuild(ByteWriter bw, ...) {
	va_list args;
	va_start(args, bw);
	while (1) {
		int type = va_arg(args, int);
		switch (type) {
			case BU8:
				ByteWriterU8(&bw, (uint8_t)va_arg(args, int));
				break;
			case BU16:
				ByteWriterU16(&bw, (uint16_t)va_arg(args, int));
				break;
			case BU32:
				ByteWriterU32(&bw, (uint32_t)va_arg(args, int));
				break;
			case BU64:
				ByteWriterU64(&bw, (uint64_t)va_arg(args, int));
				break;
			case BN:
				{
				uint32_t len = (uint32_t)va_arg(args, int);
				uint8_t* buf = (uint8_t*)va_arg(args, char*);
				ByteWriterN(&bw, buf, len);
				}
				break;
			case BSN:
				{
				uint32_t len = (uint32_t)va_arg(args, int);
				uint8_t* buf = (uint8_t*)va_arg(args, char*);
				ByteWriterSN(&bw, buf, len);
				}
				break;
			case BNULL:
				goto defer;
		}
	}
defer:
	va_end(args);
	return bw;
}
#define ByteWriterBuild(...) _ByteWriterBuild(__VA_ARGS__, BNULL)
void ByteWriterFree(ByteWriter bw) { nob_sb_free(bw.sb); };

#endif /* BYTERW_IMPLEMENTATION */
