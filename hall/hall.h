#ifndef GUARD_HALL_HALL_H
#define GUARD_HALL_HALL_H

#include <uv.h>

#include "../common/types.h"
#include "../common/net/net.h"
#include "../common/net/server.h"

#include "package.h"
#include "arm.h"
#include "platform.h"

#define FIELD_EMPTY 0
#define FIELD_PACKAGE 1
#define FIELD_PLATFORM 2
#define FIELD_ARMA 3
#define FIELD_ARMB 4

typedef struct _tSim{
	tNetServer *pServer;       /// Server handle
	ULONG ulPackageCount;      /// Total package count
															// Field fields
	UBYTE ubWidth;             /// Storage width  - field array X
	UBYTE ubHeight;            /// Storage height - field array Y
	UBYTE **pFields;           /// 2D Field array
															// Platform fields
	tPlatform *pPlatforms;     /// Array of platforms
	uv_mutex_t sPlatformMutex; /// pPlatforms & ubPlatformCount Mutex
	UBYTE ubPlatformCount;     /// Length of pPlatforms
															// Arm fields
	tArm sArmA;                /// HE HE HE...
	tArm sArmB;                /// Secondary (lower) arm
} tHall;

void hallCreate(void);

void hallDestroy(void);

void hallUpdate(
	IN uv_timer_t *pTimer
);

extern tHall g_sHall;

#endif // GUARD_HALL_HALL_H
