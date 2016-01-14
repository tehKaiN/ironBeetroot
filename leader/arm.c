#include "arm.h"
#include <uv.h>
#include "leader.h"
#include "package.h"
#include "platform.h"
#include "..\common\log.h"
#include "..\common\arm.h"

void armInit(tLeaderArm *pArm, UBYTE ubId) {
	uv_mutex_init(&pArm->sMutex);
	uv_timer_init(g_sNetManager.pLoop, &pArm->sTimer);
	pArm->sTimer.data = (void *)pArm;
	memset(pArm->pCmds, 0, MAX_COMMANDS);
	pArm->ubCmdCount = 0;
	pArm->pConn = 0;
	pArm->ubFieldX = 0;
	pArm->ubFieldY = 0;
	pArm->ubId = ubId;
	pArm->ubRangeX1 = 0;
	pArm->ubRangeX2 = 0;
	pArm->ubRangeY1 = 0;
	pArm->ubRangeY2 = 0;
	pArm->ubState = 0;
	pArm->ubCmdState = ARM_CMDSTATE_IDLE;
	pArm->ubHasNewCmds = 0;
}

tLeaderArm *armGetByConn(tNetConn *pConn) {
	if(pConn == g_sLeader.sArmA.pConn)
		return &g_sLeader.sArmA;
	else if(pConn == g_sLeader.sArmB.pConn)
		return &g_sLeader.sArmB;

	logError("Client not an arm: 0x%p", pConn);
	// TODO(#3): Close client
	return 0;
}

void armPosUpdate(uv_timer_t *pTimer) {
  tPacketHead sReq;

	if(!g_sLeader.sArmA.pConn && !g_sLeader.sArmB.pConn)
		return;

  packetPrepare((tPacket*)&sReq, PACKET_GETARMPOS, sizeof(tPacketHead));

	if(g_sLeader.sArmA.pConn)
		netSend(g_sLeader.sArmA.pConn, (tPacket *)&sReq, netNopOnWrite);
	if(g_sLeader.sArmB.pConn)
		netSend(g_sLeader.sArmB.pConn, (tPacket *)&sReq, netNopOnWrite);
}

void armUpdate(uv_timer_t *pTimer) {
	static UBYTE ubLastState = 0;
	tLeaderArm *pArm;
	tLeaderPackage *pPackage;
	tLeaderPlatform *pDst;
	tLeaderPlatform *pSrc;

	// Get idle arm
	pArm = armGetIdle();
	if(!pArm) {
		if(ubLastState != 1) {
			logWrite("No idle arms");
			ubLastState = 1;
		}
		return;
	}

	// Get package in need of pickup
	pPackage = packageGetNext(pArm);
	if(!pPackage) {
		if(ubLastState != 2) {
			logWrite("No package in need to pickup");
			ubLastState = 2;
		}
		return;
	}

	// Determine destination
	logWrite("Got package");
	pSrc = pPackage->pPlatformCurr;
	if(pPackage->pPlatformHlp) {
		if(pPackage->pPlatformHlp == pSrc)
			pDst = pPackage->pPlatformDst;
		else
			pDst = pPackage->pPlatformHlp;
	}
	else
		pDst = pPackage->pPlatformDst;

	// Route way
	uv_mutex_lock(&pArm->sMutex);
	armRoute(pArm, pSrc, pDst);
	pArm->ubHasNewCmds = 1;
	uv_mutex_unlock(&pArm->sMutex);

	// Set reserve timer

	if(pArm->ubId == ARM_ID_A)
		logWrite("Made new route for arm A with %hu commands", pArm->ubCmdCount);
	else
		logWrite("Made new route for arm B with %hu commands", pArm->ubCmdCount);
	ubLastState = 0;
//	uv_timer_init(g_sNetManager.pLoop, &pArm->sTimer);
	pArm->sTimer.data = (void *)pArm;
	uv_timer_start(&pArm->sTimer, armSetRoute, 100, 500);
}

void armSetRoute(uv_timer_t *pTimer) {
	tPacketArmCommands sPacket;
	tLeaderArm *pArm;

	pArm = (tLeaderArm *)pTimer->data;
	uv_mutex_lock(&g_sLeader.sRouteMutex);
	uv_mutex_lock(&pArm->sMutex);

	// Is route clear?
	if(!armRouteCheck(pArm))
		return;

	// Reserve route
	uv_timer_stop(&pArm->sTimer);
	armRouteReserve(pArm);

	// Send instruction list to arm
	packetPrepare(
		(tPacket *)&sPacket, PACKET_SETARMCOMMANDS, sizeof(tPacketArmCommands)
	);
	memcpy(sPacket.pCmds, pArm->pCmds, MAX_COMMANDS);
	sPacket.ubCmdCount = pArm->ubCmdCount;
	pArm->ubHasNewCmds = 0;

	uv_mutex_unlock(&pArm->sMutex);
	uv_mutex_unlock(&g_sLeader.sRouteMutex);
	netSend(pArm->pConn, (tPacket *)&sPacket, netNopOnWrite);
	pArm->ubCmdState = ARM_CMDSTATE_NEW;

	if(pArm->ubId == ARM_ID_A)
		logSuccess("Sent new route to arm A");
	else
		logSuccess("Sent new route to arm B");
}

tLeaderArm *armGetIdle(void) {
	if(rand() > RAND_MAX>>1) {
		// A, then B
		if(
			g_sLeader.sArmA.ubCmdState == ARM_CMDSTATE_IDLE &&
			!g_sLeader.sArmA.ubHasNewCmds
		)
			return &g_sLeader.sArmA;
		if(
			g_sLeader.sArmB.ubCmdState == ARM_CMDSTATE_IDLE &&
			!g_sLeader.sArmB.ubHasNewCmds
		)
			return &g_sLeader.sArmB;
	}
	else {
		// B, then A
		if(
			g_sLeader.sArmB.ubCmdState == ARM_CMDSTATE_IDLE &&
			!g_sLeader.sArmB.ubHasNewCmds
		)
			return &g_sLeader.sArmB;
		if(
			g_sLeader.sArmA.ubCmdState == ARM_CMDSTATE_IDLE &&
			!g_sLeader.sArmA.ubHasNewCmds
		)
			return &g_sLeader.sArmA;
	}
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

void armAddCmd(UBYTE *pCmds, UBYTE *pCmdCount, UBYTE ubCmd) {
	pCmds[*pCmdCount] = ubCmd;
	++*pCmdCount;
}

void armRoute(tLeaderArm *pArm, tLeaderPlatform *pSrc, tLeaderPlatform *pDst) {
	UBYTE ubX, ubY;

	ubX=pArm->ubFieldX;
	ubY=pArm->ubFieldY;

	pArm->ubCmdCount=0;

	// Move to source X
	while(ubX != pSrc->ubX){
		if(ubX > pSrc->ubX){
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_W);
			ubX-=1;
		}
		else{
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_E);
			ubX+=1;
		}
	}
	// Move to source Y
	while(ubY != pSrc->ubY){
		if(ubY > pSrc->ubY){
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_N);
			ubY-=1;
		}
		else{
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_S);
			ubY+=1;
		}
	}
	// Open-lower-close-highen
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_OPEN);
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_LOWER);
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_CLOSE);
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_HIGHEN);

	// Move to dest X
	while(ubX != pDst->ubX){
		if(ubX> pDst->ubX){
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_W);
			ubX-=1;
		}
		else{
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_E);
			ubX+=1;
		}
	}

	// Move to dest Y
	while(ubY != pDst->ubY){
		if(ubY>pDst->ubY){
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_N);
			ubY-=1;
		}
		else{
			armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_MOVE_S);
			ubY+=1;
		}
	}

	// Lower-open-highen-close-end
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_LOWER);
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_OPEN);
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_HIGHEN);
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_CLOSE);
	armAddCmd(pArm->pCmds, &pArm->ubCmdCount, ARM_CMD_END);
}

UBYTE armRouteCheck(tLeaderArm *pArm) {
	UBYTE ubX, ubY, i;

	ubX = pArm->ubFieldX;
	ubY = pArm->ubFieldY;
	for(i=0; i<pArm->ubCmdCount; i++) {
		// Update position
		switch (pArm->pCmds[i]){
			case ARM_CMD_MOVE_N:
				ubY--;
				break;
			case ARM_CMD_MOVE_S:
				ubY++;
				break;
			case ARM_CMD_MOVE_E:
				ubX++;
				break;
			case ARM_CMD_MOVE_W:
				ubX--;
				break;
		}
		// Check if field is reserved
		logWrite("Pos %hu,%hu: %hu", ubX, ubY, g_sLeader.pFields[ubX][ubY]);
		if(g_sLeader.pFields[ubX][ubY])
			return 0;
	}
	return 1;
}

void armRouteReserve(tLeaderArm *pArm) {
	UBYTE ubX, ubY, i;

	ubX = pArm->ubFieldX;
	ubY = pArm->ubFieldY;
	for(i=0; i<pArm->ubCmdCount; i++) {
		// Update position
		switch (pArm->pCmds[i]){
			case ARM_CMD_MOVE_N:
				ubY--;
				break;
			case ARM_CMD_MOVE_S:
				ubY++;
				break;
			case ARM_CMD_MOVE_E:
				ubX++;
				break;
			case ARM_CMD_MOVE_W:
				ubX--;
				break;
		}
		g_sLeader.pFields[ubX][ubY]++;
	}
}

void armReservePlatform(
	tLeaderPlatform *pSrc, tLeaderPlatform *pDst, tLeaderPlatform *pPltfReserve, 	UBYTE *pCmd, UBYTE ubCmdCount, tLeaderArm *pArm
) {
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

void armFreePlatform(tLeaderPlatform *pPltfFree){
	pPltfFree->pArmIn->ubId=ARM_ID_ILLEGAL;
	pPltfFree->pArmOut->ubId=ARM_ID_ILLEGAL;
}
