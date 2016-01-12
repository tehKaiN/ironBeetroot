#include "process.h"

void processPacket(tNetClientServer *pCS, tNetConn *pClient, tPacket *pPacket) {

	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE:
			processSetTypeResponse((tPacketSetTypeResponse*)pPacket);
			break;
		case PACKET_R_GETSENSORINFO:
			processSensorInfoResponse((tPacketSensorInfo *)pPacket);
			break;
		case PACKET_GETARMSTATE:
			processArmState();
			break;
		case PACKET_SETARMCOMMANDS:
			processSetCommands((tPacketArmCommands*)pPacket);
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

	g_sArm.ubReady |= READY_ID;
	logSuccess(
		"Identified as %s",
		g_pClientTypes[CLIENT_TYPE_CUSTOMER]
	);
}

void processSensorInfoResponse(tPacketSensorInfo *pPacket) {

	uv_mutex_lock(&g_sArm.sSensorMutex);
	g_sArm.uwCurrX = pPacket->uwX;
	g_sArm.uwCurrY = pPacket->uwY;
	g_sArm.ubState = pPacket->ubState;
	uv_mutex_unlock(&g_sArm.sSensorMutex);

	// TODO: Check if current command is fulfilled
	if(cmdIsDone()) {
		cmdStopActuators();
		cmdProcessNext();
	}
	uv_timer_start(&g_sArm.sSensorTimer, armSensorUpdate, 100, 0);
}

void processArmState(void) {
	// TODO: processArmState
}

void processSetCommands(tPacketArmCommands *pPacket) {

	if(pPacket->ubCmdCount > MAX_COMMANDS) {
		logError("Too many commands: %hu > %hu", pPacket->ubCmdCount, MAX_COMMANDS);
		return;
	}
	// TODO: processSetCommands
	uv_mutex_lock(&g_sArm.sCmdMutex);
  memcpy(g_sArm.pCmds, pPacket->pCmds, pPacket->ubCmdCount);
  g_sArm.ubCmdCount = pPacket->ubCmdCount;
  g_sArm.ubCmdCurr = 0;
  g_sArm.ubCmdState = ARM_CMDSTATE_NEW;
	uv_mutex_unlock(&g_sArm.sCmdMutex);
}
