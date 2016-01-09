#include "../common/log.h"
#include "../common/net/net.h"

#include "arm.h"
#include "hall.h"

const BYTE g_pDirDeltaX[] = {0,  0,  0,  1, -1}; // 0, N, S, E, W
const BYTE g_pDirDeltaY[] = {0, -1,  1,  0,  0}; // 0, N, S, E, W

void armInit(UBYTE ubArmId, UBYTE ubRangeBegin, UBYTE ubRangeEnd, UBYTE ubSpeed, UBYTE ubFieldPosX) {
  tArm *pArm;

  // Determine arm
  if(ubArmId == ARM_A)
		pArm = &g_sHall.sArmA;
	else if(ubArmId == ARM_B)
		pArm = &g_sHall.sArmB;
  else {
		logError("Unknown arm id: %hu", ubArmId);
		return;
  }

  // Init struct fields
  pArm->pPackage = 0;
  pArm->ubId = ubArmId;
  pArm->ubRangeBegin = ubRangeBegin;
  pArm->ubRangeEnd = ubRangeEnd;
  pArm->ubSpeed = ubSpeed;
  pArm->ubState = ARM_STATE_IDLE | ARM_STATE_OPEN | ARM_STATE_UP;
  pArm->uwX = (0xFF>>1) + (ubFieldPosX << 8);
  pArm->uwY = (0xFF>>1) + (((ubRangeBegin+ubRangeEnd)>>1) << 8);
  uv_mutex_init(&pArm->sMutex);
  pArm->ubCmdState = ARM_CMDSTATE_IDLE;

	// Start arm update timer
	uv_timer_init(g_sNetManager.pLoop, &pArm->sTimer);
	pArm->sTimer.data = pArm;
	uv_timer_start(&pArm->sTimer, armUpdate, 500, 500);

}

void armUpdate(uv_timer_t *pTimer) {
	tArm *pArm;

	pArm = pTimer->data;
	uv_mutex_lock(&pArm->sMutex);
	switch(pArm->ubCmdState) {
		case ARM_CMDSTATE_IDLE:
			// Do nothing - await new cmds
			return;
		case ARM_CMDSTATE_DONE:
			armGetNextCmd(pArm); // Current cmd done - get next cmd ready
			break;
		case ARM_CMDSTATE_NEW:
			armStartCmd(pArm);   // Start current cmd
			break;
		case ARM_CMDSTATE_BUSY:
			armProcessCmd(pArm); // Process current cmd
			break;
	}
	uv_mutex_unlock(&pArm->sMutex);
}

void armGetNextCmd(tArm *pArm) {
	if(0) { // TODO: Current command idx was last in buffer
		// No more commands available
		pArm->ubCmdState = ARM_CMDSTATE_IDLE;
		return;
	}
	// TODO: Fetch next command
	// Prepare to process current command
	pArm->ubCmdState = ARM_CMDSTATE_NEW;
}

void armStartCmd(tArm *pArm) {
	UBYTE ubCmd;
	UBYTE ubDx, ubDy;

	// TODO: ubCmd = pArm->???;
	switch(ubCmd) {
		case ARM_CMD_END:
			pArm->ubCmdState = ARM_CMDSTATE_IDLE;
			return;
		case ARM_CMD_MOVE_N:
		case ARM_CMD_MOVE_E:
		case ARM_CMD_MOVE_S:
		case ARM_CMD_MOVE_W:
			ubDx = g_pDirDeltaX[ubCmd];
			ubDy = g_pDirDeltaY[ubCmd];
			// TODO: save destination x,y in arm struct
			// TODO: boundary check
			// pArm->??? = pArm->uwX + (ubDx*0xFF) // Can't use << here
			// pArm->??? = pArm->uwY + (ubDy*0xFF) // because of signed values
			break;
		case ARM_CMD_WAIT_1S:
			// TODO: Save current timestamp
			break;
		case ARM_CMD_OPEN:
		case ARM_CMD_CLOSE:
			pArm->ubState |= ARM_STATE_CLOSED | ARM_STATE_OPEN;
			break;
		case ARM_CMD_HIGHEN:
		case ARM_CMD_LOWER:
			pArm->ubState |= ARM_STATE_DOWN | ARM_STATE_UP;
			break;
	}
	pArm->ubCmdState = ARM_CMDSTATE_BUSY;
}

void armProcessCmd(tArm *pArm) {
	UBYTE ubField;
	UBYTE ubCmd;
	UBYTE ubDx, ubDy;

	// TODO: ubCmd = pArm->???;
	switch(ubCmd) {
		case ARM_CMD_MOVE_N:
		case ARM_CMD_MOVE_E:
		case ARM_CMD_MOVE_S:
		case ARM_CMD_MOVE_W:
			if(0) { // TODO: Destination x,y not equals current x,y
				ubDx = g_pDirDeltaX[ubCmd];
				ubDy = g_pDirDeltaY[ubCmd];
				pArm->uwX += ubDx * (1 << pArm->ubSpeed);
				pArm->uwY += ubDy * (1 << pArm->ubSpeed);
				return;
			}
			break;
		case ARM_CMD_WAIT_1S:
			// TODO: get current timestamp
			if(0) // TODO: if timestamp delta < 1s
				return;
			break;
		case ARM_CMD_OPEN:
			if(pArm->pPackage) {
				if(pArm->ubState & ARM_STATE_UP) {
					// TODO: Destroy package
					logError("Package dropped from height by arm %hu", pArm->ubId);
				}
				else {
					ubField = g_sHall.pFields[pArm->uwX>>8][pArm->uwY>>8];
					if(ubField & FIELD_PLATFORM) {
						// TODO: find platform ptr
						// TODO: Drop package
					}
					else {
						logError("Package delivered not to platform by arm %hu", pArm->ubId);
						// TODO: Destroy package
					}
				}
				pArm->pPackage = 0;
			}
			break;
		case ARM_CMD_CLOSE:
			break;
		case ARM_CMD_HIGHEN:
			break;
		case ARM_CMD_LOWER:
			break;
	}
	pArm->ubCmdState = ARM_CMDSTATE_DONE;
}
