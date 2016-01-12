#include "cmd.h"
#include "../common/arm.h"
#include "../common/packet.h"

void cmdMakeStopPacket(tPacketActuators *pActuators) {

	packetPrepare(
		(tPacket *)pActuators, PACKET_SETACTUATORS, sizeof(tPacketActuators)
	);

	// Keep current height
	if(g_sArm.ubState & ARM_STATE_DOWN)
		pActuators->ubHeight = HEIGHT_DOWN;
	else
		pActuators->ubHeight = HEIGHT_UP;

	// Keep current grab state
	if(g_sArm.ubState & ARM_STATE_OPEN);
		pActuators->ubGrab = GRAB_OPEN;
	else
		pActuators->ubGrab = GRAB_CLOSE;

	// Stop motors
	pActuators->ubMotorX = MOTOR_STOP;
	pActuators->ubMotorY = MOTOR_STOP;
}

void cmdStopActuators(void) {
	tPacketActuators *sPacket;
	cmdMakeStopPacket(&sPacket);
	netSend(&g_sArm.pClient.sSrvConn, (tPacket *)pPacket, netNopOnWrite);
}

UBYTE cmdGetNext(void) {
	UBYTE ubCmd;

	uv_mutex_lock(g_sArm.sCmdMutex);
	if(g_sArm.ubCmdState == ARM_CMDSTATE_IDLE) {
		uv_mutex_unlock(g_sArm.sCmdMutex);
		return 0;
	}

	++g_sArm.ubCmdCurr;
	ubCmd = g_sArm.pCmds[g_sArm.ubCmdCurr];
	if(g_sArm.ubCmdCurr >= g_sArm.ubCmdCount || ubCmd == ARM_CMD_END) {
		g_sArm.ubCmdState = ARM_CMDSTATE_IDLE;
		uv_mutex_unlock(g_sArm.sCmdMutex);
		return 0;
	}

	g_sArm.ubCmdState = ARM_CMDSTATE_NEW;
	uv_mutex_unlock(g_sArm.sCmdMutex);
	return 1;
}

void cmdProcessNext() {
	tPacketActuators sActuators;
	UBYTE ubCmd;

	ubCmd = cmdGetNext();
	if(!ubCmd)
		return;


	switch(ubCmd) {
		case ARM_CMD_CLOSE:
			break;
		case ARM_CMD_OPEN:
			break;
		case ARM_CMD_HIGHEN:
			break;
		case ARM_CMD_LOWER:
			break;
		case ARM_CMD_MOVE_E:
			break;
		case ARM_CMD_MOVE_N:
			break;
		case ARM_CMD_MOVE_S:
			break;
		case ARM_CMD_MOVE_W:
			break;
	}
}
