#ifndef GUARD_LEADER_ARM_H
#define GUARD_LEADER_ARM_H

#include "../common/types.h"
#include <uv.h>
#include "platform.h"
struct _tLeaderPlatform;

/// Arm commands
#define ARM_CMD_MOVE_N  1
#define ARM_CMD_MOVE_S  2
#define ARM_CMD_MOVE_E  3
#define ARM_CMD_MOVE_W  4
#define ARM_CMD_WAIT_1S 5
#define ARM_CMD_OPEN    6
#define ARM_CMD_CLOSE   7
#define ARM_CMD_LOWER   8
#define ARM_CMD_HIGHEN  9
/// Following command makes arm idle (awaiting new command set)
#define ARM_CMD_END     0

#define ARM_STATE_MOVING  1
#define ARM_STATE_LOW     2
#define ARM_STATE_HIGH    4
#define ARM_STATE_OPEN    8
#define ARM_STATE_CLOSED 16

// NOTE: Each arm has half of helper platofrms to leave packages at

typedef struct _tLeaderArm{
	                  // Movement range fields
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
	OUT UBYTE *pCmdList,
	OUT UBYTE *pCmdCount
);

#endif // GUARD_LEADER_ARM_H

