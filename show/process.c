#include "process.h"
#include "../common/log.h"
#include "show.h"

void processHallPacket(
	tNetClientServer *pCS, tNetConn *pConn, tPacket *pPacket
) {
	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE:
			processSetTypeResponse((tPacketSetTypeResponse*)pPacket);
			g_sShow.ubReadyHall |= READY_HALL_ID;
			break;
		case PACKET_R_GETPLATFORMLIST:
			processPlatformList((tPacketPlatformList *)pPacket);
			break;
		case PACKET_R_GETARMPOSPREC:
			processArmPosPrec((tPacketArmPos *)pPacket);
		default:
			logWarning("Unknown packet: %hu", pPacket->sHead.ubType);
			logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}
}

void processLeaderPacket(
	tNetClientServer *pCS, tNetConn *pConn, tPacket *pPacket
) {
	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE:
			processSetTypeResponse((tPacketSetTypeResponse*)pPacket);
			g_sShow.ubReadyLeader |= READY_LEADER_ID;
			break;
		default:
			logWarning("Unknown packet: %hu", pPacket->sHead.ubType);
			logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}
}

void processSetTypeResponse(tPacketSetTypeResponse *pResponse) {
	if(!pResponse->ubIsOk) {
		logError("Identification failed");
		// TODO(#3): Close connection
		// ...
		return;
	}

	logSuccess(
		"Identified as %s",
		g_pClientTypes[CLIENT_TYPE_CUSTOMER]
	);
}

void processPlatformList(tPacketPlatformList *pPacket) {
	UBYTE i;

	if(g_sShow.ubPlatformCount)
		memFree(g_sShow.pPlatforms);

	g_sShow.ubHallHeight = pPacket->ubHallHeight;
	g_sShow.ubHallWidth = pPacket->ubHallWidth;
	g_sShow.ubPlatformCount = pPacket->ubPlatformCount;
	for(i = 0; i != g_sShow.ubPlatformCount; ++i) {
		g_sShow.pPlatforms[i].ubId = pPacket->pPlatforms[i].ubId;
		g_sShow.pPlatforms[i].ubType = pPacket->pPlatforms[i].ubType;
		g_sShow.pPlatforms[i].ubX = pPacket->pPlatforms[i].ubX;
		g_sShow.pPlatforms[i].ubY = pPacket->pPlatforms[i].ubY;
	}
	g_sShow.ubReadyHall = READY_HALL_FIELDS;
}

void processArmPosPrec(tPacketArmPosPrec *pPos) {
	g_sShow.sArmA.uwX = pPos->uwX;
	g_sShow.sArmA.uwX = pPos->uwY;
}



