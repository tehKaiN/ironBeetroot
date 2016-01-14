#include "arm.h"
#include <uv.h>
#include "leader.h"
#include "package.h"
#include "platform.h"
#include "..\common\log.h"
#include "..\common\arm.h"

void armUpdate(uv_timer_t *pTimer) {
	tLeaderArm *pArm;
	tLeaderPackage *pPackage;
	tLeaderPlatform *pDst;
	tLeaderPlatform *pSrc;
	tPacketArmCommands sCmdStr;
	tLeaderPlatform sPltfReserve;

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
	packetPrepare((tPacket *)&sCmdStr, PACKET_SETARMCOMMANDS,
								sizeof(tPacketArmCommands));
	armRoute(pArm, pSrc, pDst, &sCmdStr);
	packetPrepare((tPacket *)&sPltfReserve, PACKET_UPDATEPLATFORMS,
								sizeof(tLeaderPlatform));


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
	UBYTE ubDone;
	UBYTE ubWorkXGrab, ubWorkYGrab, ubWorkXDrop, ubWorkYDrop, ubWorkCountHelp;
	UBYTE ubWorkCountHelpTwo, ubWorkProxyTwo, ubWorkProxyThree, ubWorkProxy;


	ubWorkXGrab=pArm->ubFieldX;
	ubWorkYGrab=pArm->ubFieldY;
	ubWorkXDrop=pSrc->ubX;
	ubWorkYDrop=pSrc->ubY;
	ubWorkProxy=0;
	ubWorkProxyTwo=0;
	ubWorkProxyThree=0;
	ubDone=0;
	pCmdStr->ubCmdCount=0;
	while(ubDone==0) {
		// Is arm in correct line in axis X for the starting platform?
		if(ubWorkXGrab!=pSrc->ubX){
				// Change the X-axis position of the arm
            if(ubWorkXGrab>pSrc->ubX){
								armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_W);
                ubWorkXGrab-=1;
            }
            else{
                armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_E);
                ubWorkXGrab+=1;
            }
        }
		// Is arm in correct position in axis Y for the starting platform?
        if(ubWorkYGrab!=pSrc->ubY && ubWorkXGrab==pSrc->ubX){
						// Change the Y-axis position of the arm
            if(ubWorkYGrab>pSrc->ubY){
                armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_N);
                ubWorkYGrab+=1;
            }
            else{
                armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_S);
                ubWorkYGrab+=1;
            }
        }
        // Preparing for lifting the package
        if(ubWorkXGrab==pSrc->ubX && ubWorkYGrab==pSrc->ubY && ubWorkProxy==0){
						// Proxy set to avoid entering this code more than once
						// This is a safe way to conduct lowering, opening
						// Closing and lifting without weird manipulations on the structs
            ubWorkCountHelp=pCmdStr->ubCmdCount;
            ubWorkProxy+=1;
        }
        // Opening
				if(pCmdStr->ubCmdCount==ubWorkCountHelp){
					armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_OPEN);
				}
				// Lowering
        if(pCmdStr->ubCmdCount==ubWorkCountHelp+1){
					armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_LOWER);
        }
        // Closing
        if(pCmdStr->ubCmdCount==ubWorkCountHelp+2){
					armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_CLOSE);
        }
        // Lifting
        if(pCmdStr->ubCmdCount==ubWorkCountHelp+3){
					armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_HIGHEN);
        }
        // Did we lift the package?
        if(pCmdStr->ubCmdCount>=ubWorkCountHelp+4 && ubWorkProxyThree==0)
        {
           ubWorkProxyThree+=1;
        }
        // Is the current line in X-axis the same as for the final platform?
        if(ubWorkXDrop!=pDst->ubX && ubWorkProxyThree==1){
						// Change the X-axis position of the arm
					if(ubWorkXDrop>pDst->ubX){
						armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_W);
						ubWorkXDrop-=1;
					}
					else{
						armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_E);
						ubWorkXDrop+=1;
					}
        }
        // Are we in a good line in Y-axis for the final platform?
        if(ubWorkYDrop!=pDst->ubY && ubWorkXDrop==pDst->ubX
					&& ubWorkProxyThree==1){
						// Change the Y-axis position of the arm
					if(ubWorkYDrop>pDst->ubY){
						armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_N);
						ubWorkYDrop-=1;
					}
					else{
						armAddCmd(pCmdStr->pCmds, &pCmdStr->ubCmdCount, ARM_CMD_MOVE_S);
						ubWorkYDrop+=1;
					}
        }
        // Preparing for leaving the package
        if(ubWorkXDrop==pDst->ubX && ubWorkYDrop==pDst->ubY && ubWorkProxyTwo==0
					&& ubWorkProxyThree==1){
						// Proxy set to avoid entering this code more than once
						// This is a safe way to conduct lowering, opening
						// Closing and lifting without weird manipulations on the structs
					ubWorkCountHelpTwo=pCmdStr->ubCmdCount;
					ubWorkProxyTwo+=1;
        }
        // Lowering
				if(pCmdStr->ubCmdCount==ubWorkCountHelpTwo && ubWorkProxyThree==1){
					pCmdStr->ubCmdCount=+1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=ARM_CMD_LOWER;
				}
				// Opening
        if(pCmdStr->ubCmdCount==ubWorkCountHelpTwo+1 && ubWorkProxyThree==1){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=ARM_CMD_OPEN;
        }
        // Lifting
        if(pCmdStr->ubCmdCount==ubWorkCountHelpTwo+2 && ubWorkProxyThree==1){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=ARM_CMD_HIGHEN;
        }
        // Closing
        if(pCmdStr->ubCmdCount==ubWorkCountHelpTwo+3 && ubWorkProxyThree==1){
					pCmdStr->ubCmdCount+=1;
					pCmdStr->pCmds[pCmdStr->ubCmdCount]=ARM_CMD_CLOSE;
        }
        // DONE!
        if(pCmdStr->pCmds[pCmdStr->ubCmdCount]==ARM_CMD_CLOSE
					&& ubWorkProxyThree==1){
						pCmdStr->ubCmdCount+=1;
						pCmdStr->pCmds[pCmdStr->ubCmdCount]=ARM_CMD_END;
						ubDone=1;
        }

	}
}

void armReservePlatform(
		tLeaderPlatform *pSrc, tLeaderPlatform *pDst, tLeaderPlatform *pPltfReserve,
		tLeaderArm *pArm
		){
			UBYTE ubHelp;
			ubHelp=0;    /// Troubleshooting var, prevents mistakenly reporting errors
			if(pSrc->ubX == pPltfReserve->ubX && pSrc->ubY == pPltfReserve->ubY
						&& pPltfReserve->pArmIn->ubId == ARM_ID_ILLEGAL
						){
				pPltfReserve->pArmIn->ubId=pArm->ubId;
				ubHelp=1;
			}
			else if(pDst->ubX == pPltfReserve->ubX && pDst->ubY == pPltfReserve->ubY
								&& pPltfReserve->pArmOut->ubId == ARM_ID_ILLEGAL
								){
				pPltfReserve->pArmOut->ubId=pArm->ubId;
				ubHelp=1;
			}
			else if((pPltfReserve->pArmIn->ubId!=ARM_ID_ILLEGAL ||
								pPltfReserve->pArmOut->ubId!=ARM_ID_ILLEGAL) && ubHelp == 0
								){
				logWrite("Not free.");
				// TODO: Function for freeing platform
				return;
			}
			else if((pSrc->ubX != pPltfReserve->ubX || pSrc->ubY != pPltfReserve->ubY
						||	pDst->ubX != pPltfReserve->ubX || pDst->ubY != pPltfReserve->ubY
								) && ubHelp == 0
								){
				logWrite("Wrong coordinants.");
					return;
			}
		}
