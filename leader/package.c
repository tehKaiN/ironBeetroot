#include "package.h"
#include <uv.h>
#include "leader.h"
#include "platform.h"
#include "../common/mem.h"
#include "../common/log.h"

tLeaderPackage *packageGetNext(tLeaderArm *pArm) {
	UBYTE i;
	tLeaderPackage *pPackage;
	tLeaderPlatform *pSrc, *pDst, *pHlp;

	uv_mutex_lock(&g_sLeader.sPackageMutex);
	for(i = 0; i != g_sLeader.ubPackageCount; ++i) {
		pPackage = &g_sLeader.pPackages[i];

		// Is package already carried?
		if(pPackage->pArm) {
			continue;
		}

		// Take into account packages on IN and HELPER platforms
		pSrc = pPackage->pPlatformCurr;
		if(pSrc->ubType == PLATFORM_OUT)
			continue;

		// Check if source is within arm's reach
		if(!armCheckWithinRange(pArm, pSrc)) {
			continue;
		}

		// Check if destination is within arm's reach
		pDst = pPackage->pPlatformDst;
		if(armCheckWithinRange(pArm, pDst)) {
			// Busy destination shouldn't be a problem, so don't check for it
			pPackage->pPlatformHlp = 0;
			uv_mutex_unlock(&g_sLeader.sPackageMutex);
      return pPackage;
		}
		else {
			// Are there free helper platforms?
			pHlp = armGetFreeHelper(pArm);
			if(!pHlp) {
				continue;
			}
			pPackage->pPlatformHlp = pHlp;
			uv_mutex_unlock(&g_sLeader.sPackageMutex);
			return pPackage;
		}
	}
	uv_mutex_unlock(&g_sLeader.sPackageMutex);
	return 0;
}

void packageUpdate(uv_timer_t *pTimer) {
	tPacketHead sHead;

	if(!(g_sLeader.ubReady & READY_HALL))
		return;
	packetPrepare((tPacket *)&sHead, PACKET_GETPACKAGELIST, sizeof(sHead));

	netSend(&g_sLeader.pClient->sSrvConn, (tPacket *)&sHead, netNopOnWrite);
}

void packageAlloc(UBYTE ubPackageCount) {
	if(ubPackageCount) {
		g_sLeader.pPackages = memAlloc(ubPackageCount * sizeof(tLeaderPackage));
		g_sLeader.ubPackageCount = ubPackageCount;
	}
}

void packageFree(void) {
	if(g_sLeader.ubPackageCount) {
			memFree(g_sLeader.pPackages);
			g_sLeader.ubPackageCount = 0;
	}
}
