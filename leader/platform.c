#include "platform.h"
#include "../common/mem.h"
#include "leader.h"

void platformAlloc(UBYTE ubPlatformCount) {
  g_sLeader.pPlatforms = memAlloc(ubPlatformCount * sizeof(tLeaderPlatform));
  g_sLeader.ubPlatformCount = ubPlatformCount;
}

void platformFree(void) {
	if(g_sLeader.ubPlatformCount) {
		memFree(g_sLeader.pPlatforms);
		g_sLeader.ubPlatformCount = 0;
	}
}

tLeaderPlatform *platformGetById(UBYTE ubId) {
	tLeaderPlatform *pPlatform;
	UBYTE i;

	for(i = 0; i != g_sLeader.ubPlatformCount; ++i) {
	pPlatform = &g_sLeader.pPlatforms[i];
  if(g_sLeader.pPlatforms[i].ubId == ubId)
		return pPlatform;
	}
	return 0;
}
