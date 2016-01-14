#include "process.h"

#include "../common/mem.h"
#include "../common/log.h"
#include "../common/arm.h"
#include "../common/packet.h"
#include "../common/net/server.h"

#include "platform.h"
#include "hall.h"

void processProtocol(
	tNetServer *pServer, tNetConn *pClientConn, tPacket *pPacket
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
		case PACKET_SETTYPE:
			processSetType(pClientConn, (tPacketSetType *)pPacket);
			break;
		case PACKET_GETPLATFORMINFO:
			processPlatformInfo(pClientConn);
			break;
		case PACKET_UPDATEPLATFORMS:
			processUpdatePlatforms(pClientConn, (tPacketUpdatePlatforms*)pPacket);
			break;
		case PACKET_GETPLATFORMLIST:
			processPlatformList(pClientConn);
			break;
		case PACKET_GETPACKAGELIST:
      processPackageList(pClientConn);
			break;
		case PACKET_SETACTUATORS:
			processActuators(pClientConn, (tPacketActuators*)pPacket);
		default:
			logWarning("Unknown packet type: %hu", pPacket->sHead.ubType);
      logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}

	if(pClientConn->ubActive) {
		uv_read_start(pClientConn->pStream, netAllocBfr, netOnRead);
		netUpdateConnTime(pClientConn);
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
	else {
		// Set client type
		uv_mutex_lock(&pServer->sListMutex);
		pClientConn->ubType = ubNewClientType;
		uv_mutex_unlock(&pServer->sListMutex);
		logWrite(
			"Client 0x%p identified as '%s' (%hu)",
			pClientConn, g_pClientTypes[pClientConn->ubType], pClientConn->ubType
		);

		if(ubNewClientType == CLIENT_TYPE_CUSTOMER) {
			if(
				!platformReserveForClient(pClientConn, PLATFORM_IN,  0) ||
				!platformReserveForClient(pClientConn, PLATFORM_OUT, 0)
			) {
				logWarning("No more platforms for client %p", pClientConn);
				sResponse.ubIsOk = 0;
			}
		}
		else if(ubNewClientType == CLIENT_TYPE_ARM) {
			logWrite("Additional arm ID: %hu", pPacket->ubExtra);
			if(pPacket->ubExtra == ARM_ID_A) {
				if(g_sHall.sArmA.pConn) {
					logError("Arm A already logged in");
					// TODO(#3): close client
					return;
				}
				g_sHall.sArmA.pConn = pClientConn;
				logSuccess("Logged as Arm A");
			}
			else if(pPacket->ubExtra == ARM_ID_B) {
				if(g_sHall.sArmB.pConn) {
					logError("Arm B already logged in");
					// TODO(#3): close client
					return;
				}
				g_sHall.sArmB.pConn = pClientConn;
				logSuccess("Logged as Arm B");
			}
			else {
				logError("Unknown arm id: %hu", pPacket->ubExtra);
				// TODO(#3): close client
				return;
			}
		}
	}

	netSend(pClientConn, (tPacket*)&sResponse, 0);
	if(!sResponse.ubIsOk) {
		// TODO(#3): Close client
	}
}

void processPlatformInfo(tNetConn *pClientConn) {
	UBYTE ubClientType;
	tNetServer *pServer;

	pServer = &pClientConn->pClientServer->sServer;

	uv_mutex_lock(&pServer->sListMutex);
	ubClientType = pClientConn->ubType;
	uv_mutex_unlock(&pServer->sListMutex);

	switch(ubClientType) {
		case CLIENT_TYPE_CUSTOMER:
			processPlatformInfoGiveTake(pClientConn);
			break;
		case CLIENT_TYPE_LEADER: {
			// TODO: processPlatformInfo: CLIENT_TYPE_LEADER
			logWarning("TODO: CLIENT_TYPE_LEADER");
			} break;
		case CLIENT_TYPE_SHOW: {
			// TODO: processPlatformInfo: CLIENT_TYPE_SHOW
			logWarning("TODO: CLIENT_TYPE_SHOW");
			} break;
		default:
			if(pClientConn->ubType >= CLIENT_TYPES)
				logError(
					"Client 0x%p type too high (%hu)",
					pClientConn, pClientConn->ubType
				);
			else
				logError(
					"Unauthorized platform info poll by client 0x%p (%s)",
					pClientConn, g_pClientTypes[pClientConn->ubType]
				);
			// TODO(#3): Close client
	}
}

void processPlatformInfoGiveTake(tNetConn *pClientConn) {
	tPacketPlatformInfo sResponse;
	tPlatform *pPlatform;
	UBYTE i;

	// Prepare packet
	packetPrepare(
		(tPacket*)&sResponse, PACKET_R_GETPLATFORMINFO, sizeof(tPacketPlatformInfo)
	);

	// Prepare list of all other platform ids
	sResponse.ubDestCount = 0;
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		pPlatform = &g_sHall.pPlatforms[i];
		if(
			pPlatform->ubType == PLATFORM_OUT &&
			pPlatform->pOwner &&
			pPlatform->pOwner != pClientConn
		) {
			sResponse.pDestList[sResponse.ubDestCount] = pPlatform->ubId;
			++sResponse.ubDestCount;
		}
	}

	netSend(pClientConn, (tPacket*)&sResponse, 0);
}

void processUpdatePlatforms(
	tNetConn *pClientConn, tPacketUpdatePlatforms *pReq
) {
	tPacketUpdatePlatformsResponse sResponse;
	tPlatform *pPlatform;

	if(!hallCheckClient(pClientConn, CLIENT_TYPE_CUSTOMER))
		return;

	// Prepare response
	packetPrepare(
		(tPacket*)&sResponse, PACKET_R_UPDATEPLATFORMS,
		sizeof(tPacketUpdatePlatformsResponse)
	);

	// Is there something to grab from OUT platform?
	pPlatform = platformGetByClient(pClientConn, PLATFORM_OUT);
	if(!pPlatform) {
		logError("No recv platform for client 0x%p", pClientConn);
		// TODO(#3): Close client
		// ...
		return;
	}
	if(pPlatform->pPackage) {
		sResponse.ubGrabbed = 1;
		packageDestroy(pPlatform->pPackage);
	}
	else
		sResponse.ubGrabbed = 0;

	// Can package be dropped on IN platform?
	pPlatform = platformGetByClient(pClientConn, PLATFORM_IN);
	if(!pPlatform) {
		logError("No send platform for client 0x%p", pClientConn);
		// TODO(#3): Close client
		// ...
		return;
	}
	if(!pPlatform->pPackage && packageCreate(pPlatform, 1, pReq->ubPlatformDst)) {
		sResponse.ubPlaced = 1;
		logWrite("placed on platform 0x%p", pPlatform);
	}
	else
		sResponse.ubPlaced = 0;

	netSend(pClientConn, (tPacket*)&sResponse, netNopOnWrite);
}

void processPlatformList(tNetConn *pClientConn) {
	UBYTE i;
  tPacketPlatformList sResponse;

	if(!hallCheckClient(pClientConn, CLIENT_TYPE_LEADER))
		return;

	logWrite("Got platform list request from leader 0x%p", pClientConn);

  packetPrepare(
		(tPacket*)&sResponse, PACKET_R_GETPLATFORMLIST, sizeof(tPacketPlatformList)
	);

	uv_mutex_lock(&g_sHall.sPlatformMutex);
	sResponse.ubHallHeight = g_sHall.ubHeight;
	sResponse.ubHallWidth = g_sHall.ubWidth;
	sResponse.ubPlatformCount = g_sHall.ubPlatformCount;
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		sResponse.pPlatforms[i].ubId = g_sHall.pPlatforms[i].ubId;
		sResponse.pPlatforms[i].ubX = g_sHall.pPlatforms[i].ubFieldX;
		sResponse.pPlatforms[i].ubY = g_sHall.pPlatforms[i].ubFieldY;
		sResponse.pPlatforms[i].ubType = g_sHall.pPlatforms[i].ubType;
	}
	uv_mutex_unlock(&g_sHall.sPlatformMutex);

	netSend(pClientConn, (tPacket*)&sResponse, netNopOnWrite);
}

void processPackageList(tNetConn *pClientConn) {
	UBYTE i;
	UBYTE ubPackageCount;
	tPacketPackageList sResponse;
	tPackage *pPackage;

	packetPrepare(
		(tPacket *)&sResponse, PACKET_R_GETPACKAGELIST, sizeof(tPacketPackageList)
	);

	ubPackageCount = 0;
	// Iterate through platforms
	uv_mutex_lock(&g_sHall.sPlatformMutex);
	for(i = 0; i != g_sHall.ubPlatformCount; ++i) {
		pPackage = g_sHall.pPlatforms[i].pPackage;
		if(pPackage) {
      sResponse.pPackages[ubPackageCount].ubId = pPackage->ulIdx;
      sResponse.pPackages[ubPackageCount].ubPlatformCurrId = g_sHall.pPlatforms[i].ubId;
      sResponse.pPackages[ubPackageCount].ubPlatformDestId = g_sHall.pPlatforms[i].pPackage->pDest->ubId;
      sResponse.pPackages[ubPackageCount].ubPosType = PACKAGE_POS_PLATFORM;

      ++ubPackageCount;
		}
	}
	uv_mutex_unlock(&g_sHall.sPlatformMutex);

	// Add packages grabbed by arms
	if(g_sHall.sArmA.pPackage) {
		sResponse.pPackages[ubPackageCount].ubId = g_sHall.sArmA.pPackage->ulIdx;
		sResponse.pPackages[ubPackageCount].ubPosType = PACKAGE_POS_ARMA;
		++ubPackageCount;
	}

	if(g_sHall.sArmB.pPackage) {
		sResponse.pPackages[ubPackageCount].ubId = g_sHall.sArmB.pPackage->ulIdx;
		sResponse.pPackages[ubPackageCount].ubPosType = PACKAGE_POS_ARMB;
		++ubPackageCount;
	}

	netSend(pClientConn, (tPacket*)&sResponse, netNopOnWrite);
}

void processActuators(tNetConn *pClientConn, tPacketActuators* pPacket) {
	tHallArm *pArm;

	// Determine arm
  if(g_sHall.sArmA.pConn == pClientConn)
		pArm = &g_sHall.sArmA;
	else if(g_sHall.sArmB.pConn == pClientConn)
		pArm = &g_sHall.sArmB;
	else {
		logError("Unknown arm client: 0x%p", pClientConn);
		return;
	}

	// Move arm on X
	uv_mutex_lock(&pArm->sMutex);
	if(pPacket->ubMotorX == MOTOR_PLUS) {
		pArm->uwX += 1 << pArm->ubSpeed;
	}
	else if(pPacket->ubMotorX == MOTOR_MINUS) {
		pArm->uwX -= 1 << pArm->ubSpeed;
	}

	// Move arm on Y
	if(pPacket->ubMotorX == MOTOR_PLUS) {
		pArm->uwY += 1 << pArm->ubSpeed;
	}
	else if(pPacket->ubMotorX == MOTOR_MINUS) {
		pArm->uwY -= 1 << pArm->ubSpeed;
	}

	// Change grab state
	if(pPacket->ubGrab == GRAB_CLOSE) {
		if((pArm->ubState & ARM_STATE_GRAB_MASK) == ARM_STATE_GRABMOVE)
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_CLOSED;
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_GRABMOVE;
	}
	else if(pPacket->ubGrab == GRAB_OPEN) {
		if((pArm->ubState & ARM_STATE_GRAB_MASK) == ARM_STATE_GRABMOVE)
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_OPEN;
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_GRABMOVE;
	}

	// Change grab height
	if(pPacket->ubHeight == HEIGHT_DOWN) {
		if((pArm->ubState & ARM_STATE_MOVEV_MASK) == ARM_STATE_MOVEV)
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_DOWN;
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_MOVEV;
	}
	else if(pPacket->ubHeight == HEIGHT_UP) {
		if((pArm->ubState & ARM_STATE_MOVEV_MASK) == ARM_STATE_MOVEV)
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_UP;
		else
			pArm->ubState = (pArm->ubState & (0xFF ^ ARM_STATE_GRAB_MASK)) | ARM_STATE_MOVEV;
	}

	uv_mutex_unlock(&pArm->sMutex);
}

UBYTE _hallCheckClient(
	tNetConn *pClientConn, UBYTE ubType, const char *szFnName
) {
	if(pClientConn->ubType == ubType)
		return 1;

	logError(
		"Access denied for client 0x%p (%s) @ %s",
		pClientConn, g_pClientTypes[pClientConn->ubType], szFnName
	);
	// TODO(#3): Close client
	// ...

	return 0;
}
