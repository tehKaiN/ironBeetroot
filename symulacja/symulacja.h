#ifndef SYMULACJA_H
#define SYMULACJA_H

#include <uv.h>

#include "../common/types.h"

#include "package.h"
#include "arm.h"
#include "platform.h"

#define FIELD_EMPTY 0
#define FIELD_PACKAGE 1
#define FIELD_PLATFORM 2
#define FIELD_ARMA 3
#define FIELD_ARMB 4

typedef struct _tSim{
	ULONG ulPackageCount;  /// Total package count
	UBYTE ubPlatformCount; /// Platform count
	UBYTE ubWidth;         /// Storage width  - field array X
	UBYTE ubHeight;        /// Storage height - field array Y
	UBYTE **pFields;       /// 2D Field array
	tPlatform *pPlatforms; /// Array of platforms
	tArm sArmA;            /// HE HE HE...
	tArm sArmB;            /// Secondary (lower) arm
} tSim;

void simCreate(void);

void simDestroy(void);

void simUpdate(
	IN uv_timer_t *pTimer
);

extern tSim g_sSim;

#endif // SYMULACJA_H
