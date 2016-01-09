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

#define PACKET_RESPONSE          128

/// Illegal packet
#define PACKET_INVALID           0

/// Common packets
#define PACKET_SETTYPE           1
#define PACKET_HEARTBEAT         2
#define PACKET_DISCONNECT        3

#define PACKET_R_SETTYPE         (PACKET_RESPONSE | PACKET_SETTYPE)

/// Nadanie & odbior packets
#define PACKET_GETPLATFORMLIST   4
#define PACKET_GETPLATFORMSTATE  5
#define PACKET_GETPLATFORMINFO   6
#define PACKET_ADDPACKAGE        7
#define PACKET_GRABPACKAGE       8

#define PACKET_R_GETPLATFORMLIST  (PACKET_RESPONSE | PACKET_GETPLATFORMLIST)
#define PACKET_R_GETPLATFORMSTATE (PACKET_RESPONSE | PACKET_GETPLATFORMSTATE)
#define PACKET_R_GETPLATFORMINFO  (PACKET_RESPONSE | PACKET_GETPLATFORMINFO)
#define PACKET_R_ADDPACKAGE       (PACKET_RESPONSE | PACKET_ADDPACKAGE)
#define PACKET_R_GRABPACKAGE      (PACKET_RESPONSE | PACKET_GRABPACKAGE)
// TODO: Logistyka packets
// TODO: Ramie packets
// TODO: Wizualizacja packets

/**
 * Empty packet struct
 * Acts as header for more complex packets
 */
typedef struct _tPacketHead{
	UBYTE ubType;         /// Packet type, 0 is illegal
	UBYTE ubPacketLength; /// Packet length, **including** header size
	UWORD uwRand;         /// Random value, for obfuscating encryption pattern
} tPacketHead;

typedef struct _tPacket{
	tPacketHead sHead;
	UBYTE ubData[255-sizeof(tPacketHead)];
} tPacket;

typedef struct _tPacketSetType{
	tPacketHead sHead;
	UBYTE ubClientType;
} tPacketSetType;

typedef struct _tPacketSetTypeResponse{
	tPacketHead sHead;
	UBYTE ubIsOk;
} tPacketSetTypeResponse;

/**
 * Platform data packet
 */
typedef struct _tPacketPlatformList{
	tPacketHead sHead;
	UBYTE ubPlatformCount;
} tPacketPlatformList;

typedef struct _tPacketPlatformState{
	tPacketHead sHead;
  UBYTE ubId;
  UBYTE ubHasPackage;
} tPacketPlatformState;

typedef struct _tPacketPlatformInfo{
	tPacketPlatformState sState;
  UBYTE ubX;
  UBYTE ubY;
	char szName[16];
} tPacketPlatformInfo;

void packetMakeEmpty(
	INOUT tPacket *pPacket,
	IN UBYTE ubType
);

void packetMakeSetType(
	INOUT tPacket *pPacket,
	IN UBYTE ubClientType
);

#define packetMakeAlive(pPacket) packetMakeEmpty(pPacket, PACKET_ALIVE)
#define packetMakeBye(pPacket) packetMakeEmpty(pPacket, PACKET_BYE)

#endif // GUARD_COMMON_PACKET_H
