#include "process.h"
#include "customer.h"
#include "../common/mem.h"
#include "../common/log.h"
#include "../common/packet.h"

void processPacket(tNetClientServer *pCS, tNetConn *pConn, tPacket *pPacket) {
	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE:
			processSetTypeResponse((tPacketSetTypeResponse*)pPacket);
			break;
		case PACKET_R_GETPLATFORMINFO:
			processPlatformInfoResponse((tPacketPlatformInfo*)pPacket);
			break;
		case PACKET_R_UPDATEPLATFORMS:
			processUpdatePlatformsResponse((tPacketUpdatePlatformsResponse *)pPacket);
			break;
		default:
			logWarning("Unknown packet: %hu", pPacket->sHead.ubType);
			logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}
}

void processSetTypeResponse(tPacketSetTypeResponse *pResponse) {
	if(!pResponse->ubIsOk) {
		logError("Identification failed");
		// TODO(#3): Close connection
		// ...
		return;
	}

	g_sCustomer.ubReady |= READY_ID;
	logSuccess(
		"Identified as %s",
		g_pClientTypes[CLIENT_TYPE_CUSTOMER]
	);
}

void processPlatformInfoResponse(tPacketPlatformInfo *pInfo) {
	static WORD wPrevDestCount = -1;
	if(pInfo->ubDestCount > MAX_PLATFORMS) {
		logError(
			"Too many platforms: %hu, MAX_PLATFORMS: %hu",
			pInfo->ubDestCount, MAX_PLATFORMS
		);
		// TODO(#3): Close connection
		return;
	}

	// Update destination list
	uv_mutex_lock(&g_sCustomer.sDestMutex);
	if(g_sCustomer.ubDestCount) {
		memFree(g_sCustomer.pDestList);
		g_sCustomer.ubDestCount = 0;
	}
	if(pInfo->ubDestCount) {
		g_sCustomer.ubDestCount = pInfo->ubDestCount;
		g_sCustomer.pDestList = memAlloc(g_sCustomer.ubDestCount);
		memcpy(
			g_sCustomer.pDestList, pInfo->pDestList,
			g_sCustomer.ubDestCount
		);
	}
	uv_mutex_unlock(&g_sCustomer.sDestMutex);

	if(pInfo->ubDestCount) {
		// Get ready to send new package

		if(pInfo->ubDestCount != wPrevDestCount) {
			logWrite("Destination platforms: %hu", pInfo->ubDestCount);
			if(wPrevDestCount < 1) {
				uv_mutex_lock(&g_sCustomer.sPackageMutex);
				g_sCustomer.sPackage.ubState = NEWPACKAGE_STATE_SENT;
				uv_mutex_unlock(&g_sCustomer.sPackageMutex);
				g_sCustomer.ubReady |= READY_LIST;
			}
		}
	}
	else if(pInfo->ubDestCount != wPrevDestCount) {
		logWrite("No destinations available. Standing by...");
		g_sCustomer.ubReady &= (0xFF ^ READY_LIST);
	}
	wPrevDestCount = pInfo->ubDestCount;

}

void processUpdatePlatformsResponse(tPacketUpdatePlatformsResponse *pResp) {
	static UBYTE ubPrevPlaced = 2;
	if(pResp->ubPlaced) {
		uv_mutex_lock(&g_sCustomer.sPackageMutex);
		g_sCustomer.sPackage.ubState = NEWPACKAGE_STATE_SENT;
		uv_mutex_unlock(&g_sCustomer.sPackageMutex);

		if(ubPrevPlaced != 1)
			logSuccess("Placed package");
	}
	else {
		if(ubPrevPlaced != 0)
			logWrite("Can't place package: Source platform is busy");
		uv_mutex_lock(&g_sCustomer.sPackageMutex);
		g_sCustomer.sPackage.ubState = NEWPACKAGE_STATE_RESEND;
		uv_mutex_unlock(&g_sCustomer.sPackageMutex);
	}
	ubPrevPlaced = pResp->ubPlaced;
	if(pResp->ubGrabbed)
		logWrite("Grabbed package %hu", pResp->ubRecvPackageId);
}
