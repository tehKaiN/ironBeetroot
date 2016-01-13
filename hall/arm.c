#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/packet.h"

#include "arm.h"
#include "hall.h"

const BYTE g_pDirDeltaX[] = {0,  0,  0,  1, -1}; // 0, N, S, E, W
const BYTE g_pDirDeltaY[] = {0, -1,  1,  0,  0}; // 0, N, S, E, W

void armInit(UBYTE ubArmId, UBYTE ubRangeBegin, UBYTE ubRangeEnd, UBYTE ubSpeed, UBYTE ubFieldPosX) {
  tHallArm *pArm;

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
	tHallArm *pArm;

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

