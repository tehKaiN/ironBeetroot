#include "platform.h"
#include "hall.h"

void platformInit(UBYTE ubIdx, UBYTE ubFieldX, UBYTE ubFieldY, UBYTE ubType) {
	tPlatform *pPlatform;

	pPlatform = &g_sHall.pPlatforms[ubIdx];

	// Set flag on field
	g_sHall.pFields[ubFieldX][ubFieldY] |= FIELD_PLATFORM;

	// Fill platform struct
	pPlatform->pPackage = 0;
	pPlatform->ubFieldX = ubFieldX;
	pPlatform->ubFieldY = ubFieldY;
	pPlatform->ubId = ubIdx;
	pPlatform->ubType = ubType;
	pPlatform->pOwner = 0;
}

UBYTE platformReserveForClient(tNetConn *pClientConn, UBYTE ubType, UBYTE *pId){
	UBYTE i;
	tPlatform *pPlatform;

	// Assign new platform
	uv_mutex_lock(&g_sHall.sPlatformMutex);
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		pPlatform = &g_sHall.pPlatforms[i];
		if(pPlatform->ubType == ubType && !pPlatform->pOwner) {
			pPlatform->pOwner = pClientConn;
			if(pId)
				*pId = pPlatform->ubId;
			uv_mutex_unlock(&g_sHall.sPlatformMutex);
			return 1;
		}
	}
	uv_mutex_unlock(&g_sHall.sPlatformMutex);
	return 0;
}

tPlatform *platformGetByClient(tNetConn *pClientConn, UBYTE ubType) {
	UBYTE i;
	tPlatform *pPlatform;

	uv_mutex_lock(&g_sHall.sPlatformMutex);
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		pPlatform = &g_sHall.pPlatforms[i];
		if(pPlatform->ubType == ubType && pPlatform->pOwner == pClientConn) {
			uv_mutex_unlock(&g_sHall.sPlatformMutex);
			return pPlatform;
		}
	}
	uv_mutex_unlock(&g_sHall.sPlatformMutex);
	return 0;
}

tPlatform *platformGetById(UBYTE ubId) {
	UBYTE i;
	tPlatform *pPlatform;

	// Mutex is not needed since fn doesn't depend on owner/package info
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		pPlatform = &g_sHall.pPlatforms[i];
		if(pPlatform->ubId == ubId) {
			return pPlatform;
		}
	}
	return 0;
}

tPlatform *platformGetByPos(UBYTE ubX, UBYTE ubY) {
	UBYTE i;
	tPlatform *pPlatform;

	// Mutex is not needed since fn doesn't depend on owner/package info
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		pPlatform = &g_sHall.pPlatforms[i];
		if(pPlatform->ubFieldX == ubX && pPlatform->ubFieldY == ubY) {
			return pPlatform;
		}
	}
	return 0;
}

// TODO(#7): Unreserve platforms on client close
