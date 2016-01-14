#ifndef GUARD_LEADER_ARM_H
#define GUARD_LEADER_ARM_H

#include <uv.h>
#include "../common/types.h"
#include "../common/packet.h"
#include "../common/arm.h"
#include "../common/net/net.h"
#include "platform.h"
struct _tLeaderPlatform;

// NOTE: Each arm has half of helper platofrms to leave packages at

typedef struct _tLeaderArm{
	                            // Net stuff
	tNetConn *pConn;           /// Connection handle
	UBYTE ubId;                /// See ARM_ID_* flags
	                            // Movement range fields
	UBYTE ubRangeX1;
	UBYTE ubRangeY1;
	UBYTE ubRangeX2;
	UBYTE ubRangeY2;
															// Position fields
	UBYTE ubFieldX;            /// Current X pos
	UBYTE ubFieldY;            /// Current Y pos
															// Command fields
	UBYTE ubState;             /// See ARM_STATE_* flags
	UBYTE ubCmdCount;          /// Total commands in pCmds
	UBYTE pCmds[MAX_COMMANDS]; /// Current command list
	uv_mutex_t sMutex;         /// Mutex
} tLeaderArm;

void armInit(
	IN tLeaderArm *pArm,
	IN UBYTE ubId
);

tLeaderArm *armGetByConn(
	IN tNetConn *pConn
);

void armUpdate(
	IN uv_timer_t *pTimer
);

tLeaderArm *armGetIdle(void);

UBYTE armCheckWithinRange(
	IN tLeaderArm *pArm,
	IN struct _tLeaderPlatform *pPlatform
);

struct _tLeaderPlatform *armGetFreeHelper(
	IN tLeaderArm *pArm
);

void armRoute(
	IN tLeaderArm *pArm,
	IN struct _tLeaderPlatform *pSrc,
	IN struct _tLeaderPlatform *pDst

);

UBYTE armRouteCheck(
	IN tLeaderArm *pArm,
	IN UBYTE *pCmd,
	IN UBYTE ubCmdCount
		);

void armReservePlatform(
	IN struct _tLeaderPlatform *pSrc,
	IN struct _tLeaderPlatform *pDst,
	IN struct _tLeaderPlatform *pPltfReserve,
	IN UBYTE *pCmd,
	IN UBYTE ubCmdCount,
	IN tLeaderArm *pArm
	);
// TODO: Route mutex

void armFreePlatform(
	IN struct _tLeaderPlatform *pPltfFree
	);



void armRouteSend(
	);


#endif // GUARD_LEADER_ARM_H

