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
