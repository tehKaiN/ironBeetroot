#include "process.h"
#include "../common/log.h"
#include "../common/packet.h"
#include "../common/net/server.h"
#include "leader.h"

//************************************************************Leader as server*/

void processServerPacket(
	tNetClientServer *pCS, tNetConn *pClientConn, tPacket *pPacket
) {

	if(
		pClientConn->ubType == CLIENT_TYPE_UNKNOWN &&
		pPacket->sHead.ubType != PACKET_SETTYPE
	) {
		logError("Client 0x%p doesn't want to id himself\n", pClientConn);
		netServerRmClient(pClientConn);
		return;
	}

	switch(pPacket->sHead.ubType) {
		// TODO: Arm status
		case PACKET_SETTYPE:
			processSetType(pClientConn, (tPacketSetType *)pPacket);
			break;
		// TODO: Arm status update
		// ...
		default:
			logWarning("Unknown packet: %hu", pPacket->sHead.ubType);
			logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}

}

void processSetType(tNetConn *pClientConn, tPacketSetType *pPacket) {
	tNetServer *pServer;
	UBYTE ubNewClientType;
	tPacketSetTypeResponse sResponse;

	ubNewClientType = pPacket->ubClientType;
	pServer = &pClientConn->pClientServer->sServer;

	// Prepare response
	packetPrepare(
		(tPacket*)&sResponse.sHead, PACKET_R_SETTYPE, sizeof(tPacketSetTypeResponse)
	);
	sResponse.ubIsOk = 1;

	// Sanity check
	if(ubNewClientType >= CLIENT_TYPES) {
		logError(
			"Client 0x%p attempts to set type to %hu",
			ubNewClientType
		);
		sResponse.ubIsOk = 0;
	}
	else if(pClientConn->ubType != CLIENT_TYPE_UNKNOWN) {
		logError(
			"Client 0x%p type change attempt: '%s' (%hu) -> '%s' (%hu)",
			pClientConn,
			g_pClientTypes[pClientConn->ubType], pClientConn->ubType,
			g_pClientTypes[ubNewClientType], ubNewClientType
		);
		sResponse.ubIsOk = 0;
	}
	else if(ubNewClientType != CLIENT_TYPE_ARM) {
		logError(
			"Unauthorized client type: '%s', client: 0x%p",
			g_pClientTypes[ubNewClientType], pClientConn
		);
		sResponse.ubIsOk = 0;
	}
	else {
		// Set client type
		uv_mutex_lock(&pServer->sListMutex);
		pClientConn->ubType = ubNewClientType;
		uv_mutex_unlock(&pServer->sListMutex);
		logWrite(
			"Client 0x%p identified as '%s' (%hu)",
			pClientConn, g_pClientTypes[pClientConn->ubType], pClientConn->ubType
		);
	}

	netSend(pClientConn, (tPacket*)&sResponse, 0);
	if(!sResponse.ubIsOk) {
		// TODO(#3): Close client
	}
}

//************************************************************Leader as client*/

void processClientOnConnect(tNetClient *pClient) {
	tPacketSetType sPacket;

	g_sLeader.ubReady = 0;
	platformFree();
	packageFree();

	// Send ID packet
	logWrite(
		"Sending client type: '%s\' (%u)",
		g_pClientTypes[CLIENT_TYPE_LEADER], CLIENT_TYPE_LEADER
	);
	packetMakeSetType(&sPacket, CLIENT_TYPE_LEADER);
	netSend(&pClient->sSrvConn, (tPacket*)&sPacket, netReadOnWrite);
}

void processClientPacket(
	tNetClientServer *pCS, tNetConn *pConn, tPacket *pPacket
) {
	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE:
			processSetTypeResponse((tPacketSetTypeResponse*)pPacket);
			break;
		case PACKET_R_GETPLATFORMLIST:
			processPlatformListResponse((tPacketPlatformList*)pPacket);
			break;
		case PACKET_R_GETPACKAGELIST:
			processPackageListResponse((tPacketPackageList *)pPacket);
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

	g_sLeader.ubReady |= READY_ID;
	logSuccess(
		"Identified as %s",
		g_pClientTypes[CLIENT_TYPE_LEADER]
	);

	tPacketHead sPacket;
	packetPrepare(
		(tPacket *)&sPacket, PACKET_GETPLATFORMLIST, sizeof(tPacketHead)
	);
	netSend(&g_sLeader.pClient->sSrvConn, (tPacket *)&sPacket, netNopOnWrite);
}

void processPlatformListResponse(tPacketPlatformList *pPacket) {
	tLeaderPlatform *pPlatform;
	UBYTE i;

	logSuccess("Got info about %hu platforms", pPacket->ubPlatformCount);
	if(pPacket->ubPlatformCount > MAX_PLATFORMS) {
		logError(
			"Platform count too high: %hu > %hu",
			pPacket->ubPlatformCount, MAX_PLATFORMS
		);
		return;
	}

  g_sLeader.ubHeight = pPacket->ubHallHeight;
  g_sLeader.ubWidth = pPacket->ubHallWidth;

	platformAlloc(pPacket->ubPlatformCount);

  for(i = 0; i != g_sLeader.ubPlatformCount; ++i) {
		pPlatform = &g_sLeader.pPlatforms[i];

		pPlatform->pPackage = 0;
		pPlatform->ubId = pPacket->pPlatforms[i].ubId;
		pPlatform->ubType = pPacket->pPlatforms[i].ubType;
		pPlatform->ubX = pPacket->pPlatforms[i].ubX;
		pPlatform->ubY = pPacket->pPlatforms[i].ubY;

		if(pPlatform->ubId & 1) {
			pPlatform->pArmIn = g_sLeader.pArmA;
			pPlatform->pArmOut = g_sLeader.pArmB;
		}
		else {
			pPlatform->pArmIn = g_sLeader.pArmB;
			pPlatform->pArmOut = g_sLeader.pArmA;
		}
  }

	g_sLeader.ubReady |= READY_HALL;
}

void processPackageListResponse(tPacketPackageList *pPacket) {
	UBYTE i;
	tLeaderPlatform *pPlatform;
	tLeaderPackage *pPackage;

	logWrite("Got package list");
	// Sanity check
	if(pPacket->ubPackageCount > MAX_PACKAGES) {
		logError(
			"Package count too high: %hu > %hu",
			pPacket->ubPackageCount, MAX_PACKAGES
		);
		return;
	}

	uv_mutex_lock(&g_sLeader.sPackageMutex);
	// Realloc package array
	if(g_sLeader.ubPackageCount != pPacket->ubPackageCount) {
		packageFree();
		packageAlloc(pPacket->ubPackageCount);
		logWrite("Package count: %hu", pPacket->ubPackageCount);
	}
	// Fill package list
	for(i = 0; i != pPacket->ubPackageCount; ++i) {
		pPackage = &g_sLeader.pPackages[i];
		pPlatform = platformGetById(pPacket->pPackages[i].ubPlatformDestId);
    pPackage->ulId = pPacket->pPackages[i].ubId;
    pPackage->pPlatformDst = pPlatform;
    pPackage->pPlatformHlp = 0;

    switch(pPacket->pPackages[i].ubPosType) {
			case PACKAGE_POS_ARMA:
				pPackage->pArm = g_sLeader.pArmA;
				continue;
			case PACKAGE_POS_ARMB:
				pPackage->pArm = g_sLeader.pArmB;
				continue;
			case PACKAGE_POS_PLATFORM:
				pPlatform = platformGetById(pPacket->pPackages[i].ubPlatformCurrId);
				pPackage->pPlatformDst = pPlatform;
				continue;
		}
	}
	uv_mutex_unlock(&g_sLeader.sPackageMutex);
}
