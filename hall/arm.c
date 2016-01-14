#include "arm.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/packet.h"
#include "../common/arm.h"

#include "hall.h"

void armInit(UBYTE ubArmId, UBYTE ubRangeBegin, UBYTE ubRangeEnd, UBYTE ubSpeed, UBYTE ubFieldPosX) {
  tHallArm *pArm;

  // Determine arm
  if(ubArmId == ARM_ID_A)
		pArm = &g_sHall.sArmA;
	else if(ubArmId == ARM_ID_B)
		pArm = &g_sHall.sArmB;
  else {
		logError("Unknown arm id: %hu", ubArmId);
		return;
  }

  // Init struct fields
  pArm->pConn = 0;
  pArm->pPackage = 0;
  pArm->ubId = ubArmId;
  pArm->ubRangeBegin = ubRangeBegin;
  pArm->ubRangeEnd = ubRangeEnd;
  pArm->ubSpeed = ubSpeed;
  pArm->ubState = ARM_STATE_OPEN | ARM_STATE_UP;
  pArm->uwX = (0xFF>>1) + (ubFieldPosX << 8);
  pArm->uwY = (0xFF>>1) + (((ubRangeBegin+ubRangeEnd)>>1) << 8);
  uv_mutex_init(&pArm->sMutex);

	// Start arm update timer
	uv_timer_init(g_sNetManager.pLoop, &pArm->sTimer);
	pArm->sTimer.data = pArm;
	uv_timer_start(&pArm->sTimer, armUpdate, 500, 500);

}

void armUpdate(uv_timer_t *pTimer) {
	tHallArm *pArm;
	tPlatform *pPlatform;

	pArm = pTimer->data;
	uv_mutex_lock(&pArm->sMutex);

	// Move arm on X
	if(pArm->ubMotorX == MOTOR_PLUS) {
		pArm->uwX += 1 << pArm->ubSpeed;
	}
	else if(pArm->ubMotorX == MOTOR_MINUS) {
		pArm->uwX -= 1 << pArm->ubSpeed;
	}

	// Move arm on Y
	if(pArm->ubMotorY == MOTOR_PLUS) {
		pArm->uwY += 1 << pArm->ubSpeed;
	}
	else if(pArm->ubMotorY == MOTOR_MINUS) {
		pArm->uwY -= 1 << pArm->ubSpeed;
	}

	// Change grab state
	if(pArm->ubGrab == GRAB_CLOSE) {
		if((pArm->ubState & ARM_STATE_GRAB_MASK) == ARM_STATE_GRABMOVE) {
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_CLOSED;
			pPlatform = platformGetByPos(pArm->uwX>>8, pArm->uwY>>8);
			if((pArm->ubState & ARM_STATE_DOWN) && pPlatform && pPlatform->pPackage) {
				// Grab package
				logWrite("Grabbed package from %hu,%hu", pArm->uwX>>8, pArm->uwY>>8);
				uv_mutex_lock(&g_sHall.sPackageMutex);
				uv_mutex_lock(&g_sHall.sPlatformMutex);
				pArm->pPackage = pPlatform->pPackage;
				pPlatform->pPackage = 0;
				uv_mutex_unlock(&g_sHall.sPlatformMutex);
				uv_mutex_unlock(&g_sHall.sPackageMutex);
			}
		}
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_GRABMOVE;
	}
	else if(pArm->ubGrab == GRAB_OPEN) {
		if((pArm->ubState & ARM_STATE_GRAB_MASK) == ARM_STATE_GRABMOVE) {
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_OPEN;
			if(pArm->pPackage) {
				// Drop package
				if(!(pArm->ubState & ARM_STATE_DOWN)) {
					logError(
						"Package dropped from height @%hu,%hu", pArm->uwX>>8, pArm->uwY>>8
					);
					packageDestroy(pArm->pPackage);
					uv_mutex_lock(&g_sHall.sPackageMutex);
					pArm->pPackage = 0;
					uv_mutex_unlock(&g_sHall.sPackageMutex);
				}
				else {
					pPlatform = platformGetByPos(pArm->uwX>>8, pArm->uwY>>8);
					if(!pPlatform) {
						logError(
							"Package dropped outside platform @%hu,%hu",
							pArm->uwX>>8, pArm->uwY>>8
						);
						packageDestroy(pArm->pPackage);
						uv_mutex_lock(&g_sHall.sPackageMutex);
						pArm->pPackage = 0;
						uv_mutex_lock(&g_sHall.sPackageMutex);
					}
					else {
						uv_mutex_lock(&g_sHall.sPackageMutex);
						uv_mutex_lock(&g_sHall.sPlatformMutex);
						pPlatform->pPackage = pArm->pPackage;
						pArm->pPackage = 0;
						uv_mutex_unlock(&g_sHall.sPlatformMutex);
						uv_mutex_unlock(&g_sHall.sPackageMutex);
						logWrite(
							"Package dropped on platform @%hu,%hu", pArm->uwX>>8, pArm->uwY>>8
						);
					}
				}
			}
		}
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_GRABMOVE;
	}

	// Change grab height
	if(pArm->ubHeight == HEIGHT_DOWN) {
		if((pArm->ubState & ARM_STATE_MOVEV_MASK) == ARM_STATE_MOVEV) {
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_MOVEV_MASK)) | ARM_STATE_DOWN;
			pPlatform = platformGetByPos(pArm->uwX>>8, pArm->uwY>>8);
			if(pPlatform && pPlatform->pPackage && (pArm->ubState&ARM_STATE_CLOSED)){
				logError(
					"Crushed package with closed arm @%hu,%hu",pArm->uwX>>8, pArm->uwY>>8
				);
				packageDestroy(pPlatform->pPackage);
				uv_mutex_lock(&g_sHall.sPlatformMutex);
				pPlatform->pPackage = 0;
				uv_mutex_unlock(&g_sHall.sPlatformMutex);
			}
		}
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_MOVEV_MASK)) | ARM_STATE_MOVEV;
	}
	else if(pArm->ubHeight == HEIGHT_UP) {
		if((pArm->ubState & ARM_STATE_MOVEV_MASK) == ARM_STATE_MOVEV) {
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_MOVEV_MASK)) | ARM_STATE_UP;
		}
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_MOVEV_MASK)) | ARM_STATE_MOVEV;
	}

	uv_mutex_unlock(&pArm->sMutex);
}

