#include "package.h"

#include "../common/mem.h"
#include "../common/log.h"

#include "hall.h"
#include "platform.h"

tPackage *packageCreate(tPlatform *pPlatform, UBYTE ubPriority, UBYTE ubDestId){
  tPackage *pPackage;
  tPlatform *pDestPlatform;

	uv_mutex_lock(&g_sHall.sPackageMutex);
	// Find platform
	pDestPlatform = platformGetById(ubDestId);
	if(!pDestPlatform) {
		logError("Can't find destination platform (idx: %hu)!", ubDestId);
		return 0;
	}

	// Create package
  pPackage = memAlloc(sizeof(tPackage));
  pPackage->pDest = pDestPlatform;
  pPackage->ubPriority = ubPriority;
  pPackage->ulIdx = g_sHall.ulPackageCount;
  ++g_sHall.ulPackageCount;

  // Place package on platform
  pPlatform->pPackage = pPackage;
  uv_mutex_unlock(&g_sHall.sPackageMutex);

  return pPackage;
}

void packageDestroy(tPackage *pPackage) {
	UBYTE i;
	// Remove any associations with platform (should be one)
	uv_mutex_lock(&g_sHall.sPackageMutex);
	uv_mutex_lock(&g_sHall.sPlatformMutex);
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		if(g_sHall.pPlatforms[i].pPackage == pPackage)
			g_sHall.pPlatforms[i].pPackage  = 0;
	}
	uv_mutex_unlock(&g_sHall.sPlatformMutex);

	uv_mutex_lock(&g_sHall.sArmA.sMutex);
  if(g_sHall.sArmA.pPackage == pPackage)
		g_sHall.sArmA.pPackage = 0;
	uv_mutex_unlock(&g_sHall.sArmA.sMutex);
	uv_mutex_lock(&g_sHall.sArmB.sMutex);
  if(g_sHall.sArmB.pPackage == pPackage)
		g_sHall.sArmB.pPackage = 0;
	uv_mutex_unlock(&g_sHall.sArmB.sMutex);
	uv_mutex_unlock(&g_sHall.sPackageMutex);

	// Free package
	memFree(pPackage);
}
