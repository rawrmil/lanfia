#ifndef GAME_GETIP_H
#define GAME_GETIP_H

#include "nob.h"
#include "qrcodegen.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

bool GetIpSB(Nob_String_Builder* sb);
bool GetQRCodeBitmap(Nob_String_Builder* bitmap, char* ip, int* bitmap_size);
void PrintBitmapBig(uint8_t* bitmap, int bitmap_side, bool inv);
void PrintBitmapSmall(uint8_t* bitmap, int bitmap_side, bool inv);

#endif /* GAME_GETIP_H */

#ifdef GAME_GETIP_IMPLEMENTATION

bool GetIpSB(Nob_String_Builder* sb) {
#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	char hostname[256];
	gethostname(hostname, sizeof(hostname));
	struct hostent* host = gethostbyname(hostname);
	if (host->h_addr_list[0] != NULL) {
		struct in_addr* ia = (struct in_addr*)host->h_addr_list[0];
		nob_sb_append_cstr(sb, inet_ntoa(*ia));  // Default IP
	}
	WSACleanup();
#else
	struct ifaddrs *ifaddr;
	if (getifaddrs(&ifaddr) == -1)
		return false;
	char ip[INET_ADDRSTRLEN];
	for (struct ifaddrs *i = ifaddr; i != NULL; i = i->ifa_next) {
		if (i->ifa_addr && i->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in* si = (struct sockaddr_in*)i->ifa_addr;
			inet_ntop(AF_INET, &si->sin_addr, ip, INET_ADDRSTRLEN);
			if (strcmp(i->ifa_name, "lo") != 0) {
				nob_sb_append_cstr(sb, ip);
				break;
			}
		}
	}
	freeifaddrs(ifaddr);
	return true;
#endif
}

static bool BitmapAt(uint8_t* bitmap, int bitmap_side, int y, int x) {
	if (x < 0 || x >= bitmap_side) return 0;
	if (y < 0 || y >= bitmap_side) return 0;
	return bitmap[y*bitmap_side+x];
}

void PrintBitmapBig(uint8_t* bitmap, int bitmap_side, bool inv) {
	for (int y = 0; y < bitmap_side; y++) {
		for (int x = 0; x < bitmap_side; x++) {
			printf("%s", BitmapAt(bitmap, bitmap_side, y, x) != inv ? "██" : "  ");
		}
		printf("\n");
	}
}

void PrintBitmapSmall(uint8_t* bitmap, int bitmap_side, bool inv) {
	const char p[][4] = { " ", "▄", "▀", "█"};
	for (int hy = 0; hy < bitmap_side/2+1; hy++) {
		for (int _x = 0; _x < bitmap_side*2; _x++) {
			int x = _x%bitmap_side;
			inv = _x/bitmap_side;
			uint8_t up, down;
			up = BitmapAt(bitmap, bitmap_side, hy*2+0, x) != inv;
			if (hy*2+1 < bitmap_side)
				down = BitmapAt(bitmap, bitmap_side, hy*2+1, x) != inv;
			else
				down = inv;
			uint8_t index = ((up & 1) << 1) | (down & 1);
			printf("%s", p[index]);
		}
		printf("\n");
	}
}

bool GetQRCodeBitmap(Nob_String_Builder* bitmap, char* ip, int* bitmap_size) {
	*bitmap_size = -1;

	const size_t MAX_BUFFER = qrcodegen_BUFFER_LEN_FOR_VERSION(2);
	const size_t MAX_SIZE = 4 * 2 + 17; // 4 * VERSION + 17

	uint8_t tmp[MAX_BUFFER];
	uint8_t qrc[MAX_BUFFER];

	bool success =
		qrcodegen_encodeText(
			ip,
			tmp,
			qrc,
			qrcodegen_Ecc_MEDIUM,
			1,
			2,
			qrcodegen_Mask_AUTO,
			false
		);

	if (!success)
		return false;

	int size = qrcodegen_getSize(qrc);
	for (int x = 0; x < size+2; x++) nob_da_append(bitmap, 0);
	for (int y = 0; y < size; y++) {
		nob_da_append(bitmap, 0);
		for (int x = 0; x < size; x++) {
			nob_da_append(bitmap, qrcodegen_getModule(qrc, x, y));
		}
		nob_da_append(bitmap, 0);
	}
	for (int x = 0; x < size+2; x++) nob_da_append(bitmap, 0);

	*bitmap_size = size+2;
	return true;
}

#endif /* GAME_GETIP_IMPLEMENTATION */
