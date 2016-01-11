#include "arm.h"
#include <uv.h>
#include "leader.h"
#include "package.h"
#include "platform.h"

void armUpdate(uv_timer_t *pTimer) {
	tLeaderArm *pArm;
	tLeaderPackage *pPackage;
	tLeaderPlatform *pDst;
	tLeaderPlatform *pSrc;
	UBYTE pCmdList[255];
	UBYTE ubCmdCount;

	// Get idle arm
	pArm = armGetIdle();
	if(!pArm)
		return;

	// Get package in need of pickup
	pPackage = packageGetNext(pArm);
	if(!pPackage)
		return;

	// Determine destination
	pSrc = pPackage->pPlatformCurr;
	if(pPackage->pPlatformHlp) {
		if(pPackage->pPlatformHlp == pSrc)
			pDst = pPackage->pPlatformDst;
		else
			pDst = pPackage->pPlatformHlp;
	}
	else
		pDst = pPackage->pPlatformDst;

	// Route way and reserve fields for arm
	armRoute(pArm, pSrc, pDst, pCmdList, &ubCmdCount);

	// TODO: Send instruction list to arm


}

tLeaderArm *armGetIdle(void) {

	// Is arm A idle?
  if(!(g_sLeader.pArmA->ubState & ARM_STATE_MOVING))
		return g_sLeader.pArmA;

	// Is arm B idle?
  if(!(g_sLeader.pArmB->ubState & ARM_STATE_MOVING))
		return g_sLeader.pArmB;

	return 0;
}

UBYTE armCheckWithinRange(tLeaderArm *pArm, tLeaderPlatform *pPlatform) {
	return (
		pPlatform->ubX >= pArm->ubRangeX1 && pPlatform->ubX <= pArm->ubRangeX2 &&
		pPlatform->ubY >= pArm->ubRangeY1 && pPlatform->ubY <= pArm->ubRangeY2
	);
}

tLeaderPlatform *armGetFreeHelper(tLeaderArm *pArm) {
	UBYTE i, ubWithinRange;
	tLeaderPlatform *pPlatform;

	for(i = 0; i != g_sLeader.ubPlatformCount; ++i) {
		pPlatform = &g_sLeader.pPlatforms[i];
		ubWithinRange = armCheckWithinRange(pArm, pPlatform);
		if(pPlatform->ubType == PLATFORM_HELPER && ubWithinRange) {
			return pPlatform;
		}
	}
	return 0;
}

void armRoute(
	tLeaderArm *pArm, tLeaderPlatform *pSrc, tLeaderPlatform *pDst,
	UBYTE *pCmdList, UBYTE *pCmdCount
) {
	UBYTE ubCmdCount;
	ubCmdCount = 0;
	while(pSrc->ubX != pDst->ubX && pSrc->ubY != pDst->ubY) {
		// TODO: Route
	}
	pCmdList[ubCmdCount] = ARM_CMD_END;
	*pCmdCount = ubCmdCount;
}
