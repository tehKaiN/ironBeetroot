#ifndef GUARD_SYMULACJA_ARM_H
#define GUARD_SYMULACJA_ARM_H

#include "../common/types.h"

#include "package.h"

#define ARM_UNK 0
#define ARM_A 1
#define ARM_B 2

#define ARM_STATE_ERR 0
/// 1: arm is idle, 2: arm is moving
#define ARM_STATE_IDLE 1
#define ARM_STATE_MOVING 2
/// 4: arm is highened, 8: arm is lowered, 4|8: arm is lowering/highening
#define ARM_STATE_UP 4
#define ARM_STATE_DOWN 8
/// 16: arm is closed, 32: arm is opened, 16|32: arm is opening/closing
#define ARM_STATE_CLOSED 16
#define ARM_STATE_OPEN 32

/**
 * Arm command state
 * Error - forbidden state
 * Idle - nothing to do
 * New - command process beginning
 * Busy - command realization
 * Done - ready to fetch next command
 */
#define ARM_CMDSTATE_ERR  0
#define ARM_CMDSTATE_IDLE 1
#define ARM_CMDSTATE_NEW  2
#define ARM_CMDSTATE_BUSY 3
#define ARM_CMDSTATE_DONE 4

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

typedef struct _tArm{
	UBYTE ubRangeBegin;  /// Highest accessible row
	UBYTE ubRangeEnd;    /// Lowest accessible row
	UBYTE ubId;          /// ARM_UNK || ARM_A || ARM_B
	UBYTE ubSpeed;       /// Real speed = 1 << ubSpeed
	UWORD uwX;           /// Current pos X: one field has 256 units
	UWORD uwY;           /// Current pos Y
	UBYTE ubState;       /// See ARM_STATE_* macros
	UBYTE ubCmdState;    /// See ARM_CMDSTATE_* macros
	tPackage *pPackage;  /// Currently held package
	uv_mutex_t sMutex;   /// Arm struct mutex
} tArm;

void armInit(
	IN UBYTE ubArmId,
	IN UBYTE ubRangeBegin,
	IN UBYTE ubRangeEnd,
	IN UBYTE ubSpeed,
	IN UBYTE ubFieldPosX
);

void armUpdate(
	IN tArm *pArm
);

void armGetNextCmd(
	IN tArm *pArm
);

void armStartCmd(
	IN tArm *pArm
);

void armProcessCmd(
	IN tArm *pArm
);

#endif // GUARD_SYMULACJA_ARM_H

