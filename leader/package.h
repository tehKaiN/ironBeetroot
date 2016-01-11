#ifndef GUARD_LEADER_PACKAGE_H
#define GUARD_LEADER_PACKAGE_H

#include "arm.h"

typedef struct _tLeaderPackage{
	ULONG ulId;
	struct _tLeaderArm *pArm;              /// Carrying arm
  struct _tLeaderPlatform *pPlatformCurr;/// Carrying platform
  struct _tLeaderPlatform *pPlatformDst; /// Destination platform
  struct _tLeaderPlatform *pPlatformHlp; /// Helper destination platform
} tLeaderPackage;

tLeaderPackage *packageGetNext(
	IN struct _tLeaderArm *pArm
);

void packageUpdate(
	IN uv_timer_t *pTimer
);

void packageAlloc(
	IN UBYTE ubPackageCount
);

void packageFree(void);

#endif // GUARD_LEADER_PACKAGE_H

