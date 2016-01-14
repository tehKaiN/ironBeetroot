#ifndef GUARD_HALL_ARM_H
#define GUARD_HALL_ARM_H

#include "../common/types.h"

#include "package.h"

typedef struct _tHallArm{
	tNetConn *pConn;
	UBYTE ubRangeBegin;  /// Highest accessible row
	UBYTE ubRangeEnd;    /// Lowest accessible row
	UBYTE ubId;          /// See ARM_ID_* flags
	UBYTE ubSpeed;       /// Real speed = 1 << ubSpeed
	UWORD uwX;           /// Current pos X: one field has 256 units
	UWORD uwY;           /// Current pos Y
	UBYTE ubState;       /// See ARM_STATE_* macros
	tPackage *pPackage;  /// Currently held package
	uv_mutex_t sMutex;   /// Arm struct mutex
	uv_timer_t sTimer;   /// Arm update timer
									      // Actuator state
	UBYTE ubMotorX;
	UBYTE ubMotorY;
	UBYTE ubGrab;
	UBYTE ubHeight;
} tHallArm;

void armInit(
	IN UBYTE ubArmId,
	IN UBYTE ubRangeBegin,
	IN UBYTE ubRangeEnd,
	IN UBYTE ubSpeed,
	IN UBYTE ubFieldPosX
);

void armUpdate(
	IN uv_timer_t *pTimer
);

void armGetNextCmd(
	IN tHallArm *pArm
);

void armStartCmd(
	IN tHallArm *pArm
);

void armProcessCmd(
	IN tHallArm *pArm
);

#endif // GUARD_HALL_ARM_H

