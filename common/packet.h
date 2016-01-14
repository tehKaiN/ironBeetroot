#ifndef GUARD_COMMON_PACKET_H
#define GUARD_COMMON_PACKET_H

#include "types.h"

#define MAX_PLATFORMS 50
#define MAX_PACKAGES 50
#define MAX_COMMANDS 200
#define MAX_CUSTOMERS 50

#define CLIENT_TYPES         6
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

/// Customer packets
#define PACKET_GETPLATFORMINFO   4
#define PACKET_UPDATEPLATFORMS   5

#define PACKET_R_GETPLATFORMINFO (PACKET_RESPONSE | PACKET_GETPLATFORMINFO)
#define PACKET_R_UPDATEPLATFORMS (PACKET_RESPONSE | PACKET_UPDATEPLATFORMS)

/// Leader packets
#define PACKET_GETPLATFORMLIST   6
#define PACKET_GETPACKAGELIST    7
#define PACKET_GETARMINFO        8
#define PACKET_SETARMCOMMANDS    9
#define PACKET_GETARMPOS         10

#define PACKET_R_GETPLATFORMLIST (PACKET_RESPONSE | PACKET_GETPLATFORMLIST)
#define PACKET_R_GETPACKAGELIST  (PACKET_RESPONSE | PACKET_GETPACKAGELIST)
#define PACKET_R_GETARMINFO      (PACKET_RESPONSE | PACKET_GETARMINFO)
#define PACKET_R_SETARMCOMMANDS  (PACKET_RESPONSE | PACKET_SETARMCOMMANDS)
#define PACKET_R_GETARMPOS       (PACKET_RESPONSE | PACKET_GETARMPOS)

/// Arm packets
#define PACKET_GETSENSORINFO     11
#define PACKET_SETACTUATORS      12
#define PACKET_ARMPROGRESS       13
#define PACKET_ARMIDLE           14

#define PACKET_R_GETSENSORINFO   (PACKET_RESPONSE | PACKET_GETSENSORINFO)

/// Show packets
/// Uses also PACKET_GETPLATFORMLIST
/// Uses also PACKET_GETPACKAGELIST
/// Uses also PACKET_GETARMINFO
/// Uses also PACKET_GETARMPOS

#define PACKET_GETCUSTOMERLIST   15
#define PACKET_GETARMPOSPREC     16

#define PACKET_R_GETCUSTOMERLIST (PACKET_RESPONSE | PACKET_GETCUSTOMERLIST)
#define PACKET_R_GETARMPOSPREC   (PACKET_RESPONSE | PACKET_GETARMPOSPREC)

/// Defines for PACKET_R_GETPACKAGELIST

#define PACKAGE_POS_ARMA     1
#define PACKAGE_POS_ARMB     2
#define PACKAGE_POS_PLATFORM 3

/// Defines for PACKET_SETACTUATORS

#define MOTOR_STOP 0
#define MOTOR_MINUS 1
#define MOTOR_PLUS 2
#define GRAB_CLOSE 0
#define GRAB_OPEN 1
#define HEIGHT_UP 0
#define HEIGHT_DOWN 1

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

/**
 * PACKET_SETTYPE
 */
typedef struct _tPacketSetType{
	tPacketHead sHead;
	UBYTE ubClientType;
} tPacketSetType;

/**
 * PACKET_R_SETTYPE
 */
typedef struct _tPacketSetTypeResponse{
	tPacketHead sHead;
	UBYTE ubIsOk;
} tPacketSetTypeResponse;

/**
 * PACKET_R_GETPLATFORMINFO
 * list of available destinations
 */
 typedef struct _tPacketPlatformInfo{
	tPacketHead sHead;
	UBYTE ubDestCount;
	UBYTE pDestList[MAX_PLATFORMS];
 } tPacketPlatformInfo;

 /**
  * PACKET_UPDATEPLATFORMS
  * leaves package in hall if possible
  */
typedef struct _tPacketUpdatePlatforms{
	tPacketHead sHead;
	UBYTE ubPlatformDst;
} tPacketUpdatePlatforms;

/**
 * PACKET_R_UPDATEPLATFORMS
 * Tells if package was placed on send platform
 * Brings package form receive platform
 */
typedef struct _tPacketUpdatePlatformsResponse{
	tPacketHead sHead;
  UBYTE ubPlaced;
  UBYTE ubGrabbed;
  UBYTE ubRecvPackageId;
} tPacketUpdatePlatformsResponse;

/**
 * PACKET_R_GETPLATFORMLIST
 */

typedef struct _tPacketPlatformList{
	tPacketHead sHead;
	UBYTE ubPlatformCount;
	UBYTE ubHallWidth;
	UBYTE ubHallHeight;
	struct {
		UBYTE ubId;
    UBYTE ubX;
    UBYTE ubY;
    UBYTE ubType;
	} pPlatforms[MAX_PLATFORMS];
} tPacketPlatformList;

/**
 * PACKET_R_GETPACKAGELIST
 */
typedef struct _tPacketPackageList{
	tPacketHead sHead;
  UBYTE ubPackageCount;
  struct {
		UBYTE ubId;
		UBYTE ubPosType;         /// arm A, arm B, platform
		UBYTE ubPlatformCurrId;
		UBYTE ubPlatformDestId;
  } pPackages[MAX_PACKAGES];
} tPacketPackageList;

/**
 * PACKET_R_GETARMINFO
 */
typedef struct _tPacketArmInfo{
	tPacketHead sHead;
	UBYTE ubRangeX1;
	UBYTE ubRangeY1;
	UBYTE ubRangeX2;
	UBYTE ubRangeY2;
} tPacketArmInfo;
// TODO(#9): Leader should receive it once from hall(?)
// TODO(#9): Show should receive it once from hall(?)

/**
 * PACKET_R_GETSENSORINFO
 */
typedef struct _tPacketSensorInfo{
	tPacketHead sHead;
	UWORD uwX;
	UWORD uwY;
	UBYTE ubState;
} tPacketSensorInfo;

/**
 * PACKET_SETACTUATORS
 */
typedef struct _tPacketActuators{
	tPacketHead sHead;
	UBYTE ubMotorX;
	UBYTE ubMotorY;
	UBYTE ubHeight;
	UBYTE ubGrab;
} tPacketActuators;

/**
 * PACKET_SETARMCOMMANDS
 */
typedef struct _tPacketArmCommands{
	tPacketHead sHead;
	UBYTE ubCmdCount;
	UBYTE pCmds[MAX_COMMANDS];
} tPacketArmCommands;

/**
 * PACKET_R_GETARMPOS
 */
typedef struct _tPacketArmPos{
	tPacketHead sHead;
	UBYTE ubFieldX;
	UBYTE ubFieldY;
} tPacketArmPos;
// TODO(#9): Leader should cyclically send PACKET_GETARMPOS to arm

typedef struct _tPacketArmPosPrec{
	tPacketHead sHead;
	UWORD uwX;
	UWORD uwY;
} tPacketArmPosPrec;
// TODO(#9): Show should cyclically send PACKET_GETARMPOS to hall

/**
 * PACKET_ARMPROGRESS
 */
typedef struct _tPacketArmProgress{
	tPacketHead sHead;
	UBYTE ubCmdDone;    /// Idx of completed instruction on list
} tPacketArmProgress;

/**
 * PACKET_R_GETCUSTOMERLIST
 */
typedef struct _tPacketCustomerList{
	tPacketHead sHead;
	UBYTE ubCustomerCount;          /// Total customer count
	struct {
		UBYTE ubId;                   /// Customer id
		UBYTE ubPlatformSrc;          /// Source platform id
		UBYTE ubPlatformDst;          /// Destination platform id
	} pCustomerList[MAX_CUSTOMERS];
} tPacketCustomerList;
// TODO: Show should receive it once from Hall

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
