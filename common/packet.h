#ifndef GUARD_COMMON_PACKET_H
#define GUARD_COMMON_PACKET_H

#include "types.h"

#define MAX_PLATFORMS 100

#define CLIENT_TYPES 6
#define CLIENT_TYPE_UNKNOWN  0
#define CLIENT_TYPE_ARM      1
#define CLIENT_TYPE_LEADER   2
#define CLIENT_TYPE_CUSTOMER 3
#define CLIENT_TYPE_SHOW     4
#define CLIENT_TYPE_HALL     5

#define PACKET_RESPONSE          128

/// Illegal packet
#define PACKET_INVALID           0

/// Common packets
#define PACKET_SETTYPE           1
#define PACKET_HEARTBEAT         2
#define PACKET_DISCONNECT        3

#define PACKET_R_SETTYPE         (PACKET_RESPONSE | PACKET_SETTYPE)

/// customer packets
#define PACKET_GETPLATFORMINFO   4
#define PACKET_UPDATEPLATFORMS   5

#define PACKET_R_GETPLATFORMINFO  (PACKET_RESPONSE | PACKET_GETPLATFORMINFO)
#define PACKET_R_UPDATEPLATFORMS  (PACKET_RESPONSE | PACKET_UPDATEPLATFORMS)
/// TODO(#9): leader packets
/// TODO(#9): arm packets
/// TODO(#9): show packets

//******************************************************************* STRUCTS */

/**
 * Packet header
 * Can be used to send empty packets
 */
typedef struct _tPacketHead{
	UBYTE ubType;         /// Packet type, 0 is illegal
	UBYTE ubPacketLength; /// Packet length, **including** header size
	UWORD uwRand;         /// Random value, for obfuscating encryption pattern
} tPacketHead;

/**
 * Abstract packet - common ancestor
 * Can be cast to other packets
 */
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
 * Platform info - list of available destinations
 */
 typedef struct _tPacketPlatformInfo{
	tPacketHead sHead;
	UBYTE ubDestCount;
	UBYTE pDestList[MAX_PLATFORMS];
 } tPacketPlatformInfo;

 /**
  * Update platforms packet
  * Leaves package in hall if possible
  */
typedef struct _tPacketUpdatePlatforms{
	tPacketHead sHead;
	UBYTE ubPlatformDst;
} tPacketUpdatePlatforms;

/**
 * Response to tPacketUpdatePlatforms
 * Tells if package was placed on send platform
 * Brings package form receive platform
 */
typedef struct _tPacketUpdatePlatformsResponse{
	tPacketHead sHead;
  UBYTE ubPlaced;
  UBYTE ubGrabbed;
  UBYTE ubRecvPackageId;
} tPacketUpdatePlatformsResponse;

//***************************************************************** FUNCTIONS */

void packetPrepare(
	INOUT tPacket *pPacket,
	IN UBYTE ubType,
	IN UBYTE ubSize
);

void packetMakeSetType(
	INOUT tPacketSetType *pPacket,
	IN UBYTE ubClientType
);

#define packetMakeEmpty(pPacket, ubType) \
	packetMakeHead(pPacket, ubType, sizeof(tPacketHead))
#define packetMakeAlive(pPacket) \
	packetMakeEmpty(pPacket, PACKET_ALIVE)
#define packetMakeBye(pPacket) \
	packetMakeEmpty(pPacket, PACKET_BYE)

extern const char *g_pClientTypes[CLIENT_TYPES];

#endif // GUARD_COMMON_PACKET_H
