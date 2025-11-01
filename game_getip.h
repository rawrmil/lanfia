#ifndef GAME_GETIP_H
#define GAME_GETIP_H

#include "nob.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

bool GetIpSB(Nob_String_Builder* sb);

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

#endif /* GAME_GETIP_IMPLEMENTATION */
