#ifndef GUARD_COMMON_PACKET_H
#define GUARD_COMMON_PACKET_H

#include "types.h"

#define CLIENT_TYPES 7
#define CLIENT_TYPE_UNKNOWN 0
#define CLIENT_TYPE_RAMIE 1
#define CLIENT_TYPE_LOGISTYKA 2
#define CLIENT_TYPE_NADANIE 3
#define CLIENT_TYPE_ODBIOR 4
#define CLIENT_TYPE_WIZUALIZACJA 5
#define CLIENT_TYPE_SYMULACJA 6

extern const char *g_szClientTypes[CLIENT_TYPES];

#define PACKET_INVALID 0
#define PACKET_HELLO 1
#define PACKET_ALIVE 2
#define PACKET_BYE 3

typedef struct _tPacketHead{
	UBYTE ubType;         /// Packet type, 0 is illegal
	UBYTE ubPacketLength; /// Packet length, including header size!
} tPacketHead;

typedef struct _tPacketHello{
	UBYTE ubClientType;
} tPacketHello;

typedef struct _tPacket{
	tPacketHead sHead;
	union {
		char szData[255];
		tPacketHello sHello;
	};
} tPacket;

void packetMakeEmpty(
	INOUT tPacket *pPacket,
	IN UBYTE ubType
);

void packetMakeHello(
	INOUT tPacket *pPacket,
	IN UBYTE ubClientType
);

#define packetMakeAlive(pPacket) packetMakeEmpty(pPacket, PACKET_ALIVE)
#define packetMakeBye(pPacket) packetMakeEmpty(pPacket, PACKET_BYE)

#endif // GUARD_COMMON_PACKET_H
