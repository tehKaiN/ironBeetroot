#include "../common/log.h"

#include "arm.h"
#include "symulacja.h"

void armInit(UBYTE ubArmId, UBYTE ubRangeBegin, UBYTE ubRangeEnd, UBYTE ubSpeed, UBYTE ubFieldPosX) {
  tArm *pArm;

  // Determine arm
  if(ubArmId == ARM_A)
		pArm = &g_sSim.sArmA;
	else if(ubArmId == ARM_B)
		pArm = &g_sSim.sArmB;
  else {
		logError("Unknown arm Id: %hu", ubArmId);
		return;
  }

  // Init arm
  pArm->pPackage = 0;
  pArm->ubId = ubArmId;
  pArm->ubRangeBegin = ubRangeBegin;
  pArm->ubRangeEnd = ubRangeEnd;
  pArm->ubSpeed = ubSpeed;
  pArm->ubState = ARM_STATE_IDLE | ARM_STATE_OPEN | ARM_STATE_UP;
  pArm->uwX = (0xFF>>1) + (ubFieldPosX << 8);
  pArm->uwY = (0xFF>>1) + (((ubRangeBegin+ubRangeEnd)>>1) << 8);

}
