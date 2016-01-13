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
	tPacketArmCommands *pCmdStr;

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
	armRoute(pArm, pSrc, pDst, pCmdStr);

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

/*
 * NOTE: consider code refactor:
 * while(currentY != srcY)
 * 	...;
 * while(currentX != srcX)
 * 	...;
 * open cmd;
 * lower cmd;
 * close cmd;
 * highen cmd;
 * while(currentY != dstY)
 * 	...
 * while(currentX != dstX)
 * 	...
 * lower cmd;
 * open cmd;
 * highen cmd;
 * close cmd;
 *
 * currently while(!done) makes this thing overcomplicated
 */

// NOTE: should shorten armRoute code a bit
void armAddCmd(UBYTE *pCmds, UBYTE *pCmdCount, UBYTE ubCmd) {
	pCmds[*pCmdCount] = ubCmd;
	++*pCmdCount;
}

void armRoute(
	tLeaderArm *pArm, tLeaderPlatform *pSrc, tLeaderPlatform *pDst,
	tPacketArmCommands *pCmdStr
) {
	// TODO: prefix ub for UBYTEs
	// TODO: use ARM_CMD_* defines from ../common/arm.h
	// TODO: use armAddCmd(), should shorten code a bit
	UBYTE done;
	UBYTE workXGrab, workYGrab, workXDrop, workYDrop, workCountHelp, workProxy;
	UBYTE workCountHelpTwo, workProxyTwo, workProxyThree;


	workXGrab=pArm->ubFieldX;
	workYGrab=pArm->ubFieldY;
	workXDrop=pSrc->ubX;
	workYDrop=pSrc->ubY;
	workProxy=0;
	workProxyTwo=0;
	workProxyThree=0;
	done=0;
	pCmdStr->ubCmdCount=0;
	while(done==0) {
		// Is arm in correct line in axis X for the starting platform?
		if(workXGrab!=pSrc->ubX){
				// Change the X-axis position of the arm
            if(workXGrab>pSrc->ubX){
                pCmdStr->pCmds[pCmdStr->ubCmdCount]=4;
                workXGrab-=1;
                pCmdStr->ubCmdCount+=1;
            }
            else{
                pCmdStr->pCmds[pCmdStr->ubCmdCount]=3;
                workXGrab+=1;
                pCmdStr->ubCmdCount+=1;
            }
        }
		// Is arm in correct position in axis Y for the starting platform?
        if(workYGrab!=pSrc->ubY && workXGrab==pSrc->ubX){
						// Change the Y-axis position of the arm
            if(workYGrab>pSrc->ubY){
                pCmdStr->pCmds[pCmdStr->ubCmdCount]=1;
                workYGrab-=1;
                pCmdStr->ubCmdCount+=1;
            }
            else{
                pCmdStr->pCmds[pCmdStr->ubCmdCount]=2;
                workYGrab+=1;
                pCmdStr->ubCmdCount+=1;
            }
        }
        // Preparing for lifting the package
        if(workXGrab==pSrc->ubX && workYGrab==pSrc->ubY && workProxy==0){
						// Proxy set to avoid entering this code more than once
						// This is a safe way to conduct lowering, opening
						// Closing and lifting without weird manipulations on the structs
            workCountHelp=pCmdStr->ubCmdCount;
            workProxy+=1;
        }
        // Opening
				if(pCmdStr->ubCmdCount==workCountHelp){
					// NOTE: Shouldn't ubCmdCount be increased after setting cmd?
					pCmdStr->ubCmdCount=+1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=6;
				}
				// Lowering
        if(pCmdStr->ubCmdCount==workCountHelp+1){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=8;
        }
        // Closing
        if(pCmdStr->ubCmdCount==workCountHelp+2){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=7;
        }
        // Lifting
        if(pCmdStr->ubCmdCount==workCountHelp+3){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=9;
        }
        // Did we lift the package?
        if(pCmdStr->ubCmdCount>=workCountHelp+4 && workProxyThree==0)
        {
           workProxyThree+=1;
        }
        // Is the current line in X-axis the same as for the final platform?
        if(workXDrop!=pDst->ubX && workProxyThree==1){
						// Change the X-axis position of the arm
					if(workXDrop>pDst->ubX){
						pCmdStr->pCmds[pCmdStr->ubCmdCount]=4;
						workXDrop-=1;
						pCmdStr->ubCmdCount+=1;
					}
					else{
						pCmdStr->pCmds[pCmdStr->ubCmdCount]=3;
						workXDrop+=1;
						pCmdStr->ubCmdCount+=1;
					}
        }
        // Are we in a good line in Y-axis for the final platform?
        if(workYDrop!=pDst->ubY && workXDrop==pDst->ubX && workProxyThree==1){
						// Change the Y-axis position of the arm
					if(workYDrop>pDst->ubY){
						pCmdStr->pCmds[pCmdStr->ubCmdCount]=1;
						workYDrop-=1;
						pCmdStr->ubCmdCount+=1;
					}
					else{
						pCmdStr->pCmds[pCmdStr->ubCmdCount]=2;
						workYDrop+=1;
						pCmdStr->ubCmdCount+=1;
					}
        }
        // Preparing for leaving the package
        if(workXDrop==pDst->ubX && workYDrop==pDst->ubY && workProxyTwo==0
					&& workProxyThree==1){
						// Proxy set to avoid entering this code more than once
						// This is a safe way to conduct lowering, opening
						// Closing and lifting without weird manipulations on the structs
					workCountHelpTwo=pCmdStr->ubCmdCount;
					workProxyTwo+=1;
        }
        // Lowering
				if(pCmdStr->ubCmdCount==workCountHelpTwo && workProxyThree==1){
					pCmdStr->ubCmdCount=+1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=8;
				}
				// Opening
        if(pCmdStr->ubCmdCount==workCountHelpTwo+1 && workProxyThree==1){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=6;
        }
        // Lifting
        if(pCmdStr->ubCmdCount==workCountHelpTwo+2 && workProxyThree==1){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=9;
        }
        // Closing
        if(pCmdStr->ubCmdCount==workCountHelpTwo+3 && workProxyThree==1){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=7;
        }
        // DONE!
        if(pCmdStr->pCmds[pCmdStr->ubCmdCount]==7 && workProxyThree==1){
						pCmdStr->ubCmdCount+=1;
						pCmdStr->pCmds[pCmdStr->ubCmdCount]=0;
						done=1;
        }

	}
}
