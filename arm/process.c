#include "process.h"
#include "../common/log.h"
#include "arm.h"
#include "cmd.h"

void processHallPacket(
	tNetClientServer *pCS, tNetConn *pClient, tPacket *pPacket
) {

	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE:
			processSetTypeResponse((tPacketSetTypeResponse*)pPacket);
			g_sArm.ubReady |= READY_ID_HALL;
			break;
		case PACKET_R_GETSENSORINFO:
			processSensorInfoResponse((tPacketSensorInfo *)pPacket);
			break;
		default:
			logWarning("Unknown packet: %hu", pPacket->sHead.ubType);
			logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}
}

void processLeaderPacket(
	tNetClientServer *pCS, tNetConn *pClient, tPacket *pPacket
) {

	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE:
			processSetTypeResponse((tPacketSetTypeResponse*)pPacket);
			g_sArm.ubReady |= READY_ID_LEADER;
			break;
		case PACKET_SETARMCOMMANDS:
			processSetCommands((tPacketArmCommands*)pPacket);
			break;
		case PACKET_GETARMPOS:
			processArmPos();
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

void processSensorInfoResponse(tPacketSensorInfo *pPacket) {
	tPacketArmProgress sPacket;

	uv_mutex_lock(&g_sArm.sSensorMutex);
	g_sArm.uwCurrX = pPacket->uwX;
	g_sArm.uwCurrY = pPacket->uwY;
	g_sArm.ubState = pPacket->ubState;
	uv_mutex_unlock(&g_sArm.sSensorMutex);

	if(!cmdIsDone())
		return;

	cmdStopActuators();
	cmdGoNext();

	if(g_sArm.ubCmdState == ARM_CMDSTATE_IDLE)
		return;

	// Report done cmd
	packetPrepare(
		(tPacket *)&sPacket, PACKET_ARMPROGRESS, sizeof(tPacketArmProgress)
	);
	uv_mutex_lock(&g_sArm.sCmdMutex);
	sPacket.ubCmdDone = g_sArm.ubCmdCurr-1;
	sPacket.ubX = g_sArm.uwCurrX>>8;
	sPacket.ubY = g_sArm.uwCurrY>>8;
	uv_mutex_unlock(&g_sArm.sCmdMutex);
	netSend(
		&g_sArm.pClientLeader->sSrvConn, (tPacket *)&sPacket, netNopOnWrite
	);
	logWrite(
		"Current command done, x: %hu, y: %hu", sPacket.ubX, sPacket.ubY
	);

	// Do next cmd
	cmdProcessCurr();
}

void processArmPos(void) {
	tPacketArmPos sResponse;

	packetPrepare((tPacket*)&sResponse,PACKET_R_GETARMPOS, sizeof(tPacketArmPos));
	sResponse.ubFieldX = g_sArm.uwCurrX>>8;
	sResponse.ubFieldY = g_sArm.uwCurrY>>8;
	sResponse.ubState = g_sArm.ubState;
	sResponse.ubCmdState = g_sArm.ubCmdState;

	netSend(&g_sArm.pClientLeader->sSrvConn,(tPacket *)&sResponse, netNopOnWrite);
}

void processSetCommands(tPacketArmCommands *pPacket) {

	if(pPacket->ubCmdCount > MAX_COMMANDS) {
		logError("Too many commands: %hu > %hu", pPacket->ubCmdCount, MAX_COMMANDS);
		return;
	}
	logSuccess("Received new command set");

	uv_mutex_lock(&g_sArm.sCmdMutex);
  memcpy(g_sArm.pCmds, pPacket->pCmds, pPacket->ubCmdCount);
  g_sArm.ubCmdCount = pPacket->ubCmdCount;
  g_sArm.ubCmdCurr = 0;
  g_sArm.ubCmdState = ARM_CMDSTATE_NEW;
	uv_mutex_unlock(&g_sArm.sCmdMutex);
	cmdProcessCurr();
}
