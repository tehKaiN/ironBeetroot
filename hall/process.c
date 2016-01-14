#include "process.h"

#include "../common/mem.h"
#include "../common/log.h"
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
		default:
			logWarning("Unknown packet type: %hu",
				pPacket->sHead.ubType
			);
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
  tPacketPlatformList sResponse;

  packetPrepare(&sResponse, )
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
