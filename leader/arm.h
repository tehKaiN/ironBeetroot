#ifndef GUARD_LEADER_ARM_H
#define GUARD_LEADER_ARM_H

#include "../common/types.h"
#include "../common/packet.h"
#include <uv.h>
#include "platform.h"
struct _tLeaderPlatform;





// NOTE: Each arm has half of helper platofrms to leave packages at

typedef struct _tLeaderArm{
	                  // Movement range fields
	UBYTE ubId;
	UBYTE ubRangeX1;
	UBYTE ubRangeY1;
	UBYTE ubRangeX2;
	UBYTE ubRangeY2;
	UBYTE ubState;   /// See ARM_STATE_* flags
	UBYTE ubFieldX;
	UBYTE ubFieldY;
} tLeaderArm;

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
	IN struct _tLeaderPlatform *pDst,
	IN tPacketArmCommands *pCmdStr
);

void armReservePlatform(
	IN struct _tLeaderPlatform *pSrc,
	IN struct _tLeaderPlatform *pDst,
	IN struct _tLeaderPlatform *pPltfReserve,
	IN tLeaderArm *pArm
	);

#endif // GUARD_LEADER_ARM_H

