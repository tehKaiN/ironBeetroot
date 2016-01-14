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

	uv_mutex_lock(&g_sArm.sSensorMutex);
	g_sArm.uwCurrX = pPacket->uwX;
	g_sArm.uwCurrY = pPacket->uwY;
	g_sArm.ubState = pPacket->ubState;
	uv_mutex_unlock(&g_sArm.sSensorMutex);

	if(cmdIsDone()) {
		cmdStopActuators();
		cmdGoNext();

		if(g_sArm.ubCmdState == ARM_CMDSTATE_IDLE) {
			tPacketHead sPacket;

			// Report idle state to arm
      packetPrepare((tPacket *)&sPacket, PACKET_ARMIDLE, sizeof(tPacketHead));
      netSend(
				&g_sArm.pClientLeader->sSrvConn, (tPacket *)&sPacket, netNopOnWrite
			);
		}
		else {
			tPacketArmProgress sPacket;

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

			// Do next cmd
			cmdProcessCurr();
		}
	}
}

void processSetCommands(tPacketArmCommands *pPacket) {

	if(pPacket->ubCmdCount > MAX_COMMANDS) {
		logError("Too many commands: %hu > %hu", pPacket->ubCmdCount, MAX_COMMANDS);
		return;
	}

	uv_mutex_lock(&g_sArm.sCmdMutex);
  memcpy(g_sArm.pCmds, pPacket->pCmds, pPacket->ubCmdCount);
  g_sArm.ubCmdCount = pPacket->ubCmdCount;
  g_sArm.ubCmdCurr = 0;
  g_sArm.ubCmdState = ARM_CMDSTATE_NEW;
	uv_mutex_unlock(&g_sArm.sCmdMutex);
	cmdProcessCurr();
}
