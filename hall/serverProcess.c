#include "serverProcess.h"
#include "../common/mem.h"
#include "../common/log.h"
#include "../common/packet.h"
#include "../common/net/server.h"

void serverProcessProtocol(
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
			serverProcessSetType(pClientConn, (tPacketSetType *)pPacket);
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

void serverProcessSetType(tNetConn *pClientConn, tPacketSetType *pPacket) {
	tNetServer *pServer;
	UBYTE ubNewClientType;
	tPacketSetTypeResponse sResponse;

	// Sanity check
	ubNewClientType = pPacket->ubClientType;
	pServer = &pClientConn->pClientServer->sServer;
	if(ubNewClientType >= CLIENT_TYPES) {
		logError(
			"Client 0x%p attempts to set type to %hu",
			ubNewClientType
		);
		netServerRmClient(pClientConn);
    return;
	}
	if(pClientConn->ubType != CLIENT_TYPE_UNKNOWN) {
		logError(
			"Client 0x%p type change attempt: '%s' (%hu) -> '%s' (%hu)",
			pClientConn,
			g_pClientTypes[pClientConn->ubType], pClientConn->ubType,
			g_pClientTypes[ubNewClientType], ubNewClientType
		);
		netServerRmClient(pClientConn);
		return;
	}

	uv_mutex_lock(&pServer->sListMutex);
	pClientConn->ubType = ubNewClientType;
	uv_mutex_unlock(&pServer->sListMutex);
	logWrite(
		"Client 0x%p identified as '%s' (%hu)",
		pClientConn, g_pClientTypes[pClientConn->ubType], pClientConn->ubType
	);

	packetMakeHead(
		&sResponse.sHead, PACKET_R_SETTYPE, sizeof(tPacketSetTypeResponse)
	);
	sResponse.ubIsOk = 1;
	netSend(pClientConn, (tPacket*)&sResponse, netReadAfterWrite);
}





