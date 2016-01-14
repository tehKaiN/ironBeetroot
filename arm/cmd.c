#include "cmd.h"
#include "../common/log.h"
#include "../common/arm.h"
#include "../common/packet.h"
#include "arm.h"

UBYTE cmdIsDone(void) {
	UBYTE ubCmd;
	uv_mutex_lock(&g_sArm.sCmdMutex);
	ubCmd = g_sArm.pCmds[g_sArm.ubCmdCurr];
	uv_mutex_unlock(&g_sArm.sCmdMutex);

  switch(ubCmd) {
		case ARM_CMD_OPEN:
			if((g_sArm.ubState & ARM_STATE_GRAB_MASK) == ARM_STATE_OPEN)
				return 1;
			return 0;
    case ARM_CMD_CLOSE:
			if((g_sArm.ubState & ARM_STATE_GRAB_MASK) == ARM_STATE_CLOSED)
				return 1;
			return 0;
		case ARM_CMD_HIGHEN:
			if((g_sArm.ubState & ARM_STATE_MOVEV_MASK) == ARM_STATE_UP)
				return 1;
			return 0;
		case ARM_CMD_LOWER:
			if((g_sArm.ubState & ARM_STATE_MOVEV_MASK) == ARM_STATE_DOWN)
				return 1;
			return 0;
		case ARM_CMD_MOVE_E:
		case ARM_CMD_MOVE_W:
			if(g_sArm.uwDestX == g_sArm.uwCurrX)
				return 1;
			return 0;
		case ARM_CMD_MOVE_N:
		case ARM_CMD_MOVE_S:
			if(g_sArm.uwDestY == g_sArm.uwDestY)
				return 1;
			return 0;
		case ARM_CMD_END:
			return 1;
		default:
			logError("Unknown command code: %hu", ubCmd);
  }
  return 0;
}

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
	if(g_sArm.ubState & ARM_STATE_OPEN)
		pActuators->ubGrab = GRAB_OPEN;
	else
		pActuators->ubGrab = GRAB_CLOSE;

	// Stop motors
	pActuators->ubMotorX = MOTOR_STOP;
	pActuators->ubMotorY = MOTOR_STOP;
}

void cmdStopActuators(void) {
	tPacketActuators sPacket;

	cmdMakeStopPacket(&sPacket);
	netSend(&g_sArm.pClientHall->sSrvConn, (tPacket *)&sPacket, netNopOnWrite);
}

UBYTE cmdGoNext(void) {
	UBYTE ubCmd;

	uv_mutex_lock(&g_sArm.sCmdMutex);
	if(g_sArm.ubCmdState == ARM_CMDSTATE_IDLE) {
		uv_mutex_unlock(&g_sArm.sCmdMutex);
		return 0;
	}

	++g_sArm.ubCmdCurr;
	ubCmd = g_sArm.pCmds[g_sArm.ubCmdCurr];
	if(g_sArm.ubCmdCurr >= g_sArm.ubCmdCount || ubCmd == ARM_CMD_END) {
		g_sArm.ubCmdState |= ARM_CMDSTATE_IDLE;
		uv_mutex_unlock(&g_sArm.sCmdMutex);
		return 0;
	}

	g_sArm.ubCmdState = ARM_CMDSTATE_NEW;
	uv_mutex_unlock(&g_sArm.sCmdMutex);
	return ubCmd;
}

void cmdProcessCurr(void) {
	tPacketActuators sPacket;

	// Do nothing unless command requires it
	cmdMakeStopPacket(&sPacket);

	uv_mutex_lock(&g_sArm.sCmdMutex);
	if(g_sArm.ubCmdState == ARM_CMDSTATE_IDLE) {
		uv_mutex_unlock(&g_sArm.sCmdMutex);
		return;
	}
	switch(g_sArm.pCmds[g_sArm.ubCmdCurr]) {
		case ARM_CMD_OPEN:
			sPacket.ubGrab = GRAB_OPEN;
			break;
		case ARM_CMD_CLOSE:
			sPacket.ubGrab = GRAB_CLOSE;
			break;
		case ARM_CMD_HIGHEN:
			sPacket.ubHeight = HEIGHT_UP;
			break;
		case ARM_CMD_LOWER:
			sPacket.ubHeight = HEIGHT_DOWN;
			break;
		case ARM_CMD_MOVE_N:
			sPacket.ubMotorY = MOTOR_MINUS;
			g_sArm.uwDestY -= 256;
			break;
		case ARM_CMD_MOVE_S:
			sPacket.ubMotorY = MOTOR_PLUS;
			g_sArm.uwDestY += 256;
			break;
		case ARM_CMD_MOVE_E:
			sPacket.ubMotorX = MOTOR_PLUS;
			g_sArm.uwDestX += 256;
			break;
		case ARM_CMD_MOVE_W:
			sPacket.ubMotorY = MOTOR_MINUS;
			g_sArm.uwDestX -= 256;
			break;
	}
	uv_mutex_unlock(&g_sArm.sCmdMutex);

	netSend(&g_sArm.pClientHall->sSrvConn, (tPacket *)&sPacket, netNopOnWrite);
}
