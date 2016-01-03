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

typedef struct _tArm{
	UBYTE ubRangeBegin;  /// Highest accessible row
	UBYTE ubRangeEnd;    /// Lowest accessible row
	UBYTE ubId;          /// ARM_UNK || ARM_A || ARM_B
	UBYTE ubSpeed;       /// Real speed = 1 << ubSpeed
	UWORD uwX;           /// Current pos X: one field has 256 units
	UWORD uwY;           /// Current pos Y
	UBYTE ubState;       /// See ARM_STATE_* macros
	tPackage *pPackage;  /// Currently held package
} tArm;

void armInit(
	IN UBYTE ubArmId,
	IN UBYTE ubRangeBegin,
	IN UBYTE ubRangeEnd,
	IN UBYTE ubSpeed,
	IN UBYTE ubFieldPosX
);

#endif // GUARD_SYMULACJA_ARM_H

