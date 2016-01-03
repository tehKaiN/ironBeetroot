#include "serverProcess.h"
#include "../common/mem.h"
#include "../common/log.h"
#include "../common/packet.h"
#include "../common/net/server.h"

void serverProcessProtocol(tNetServer *pServer, tNetConn *pClientConn, tPacket *pPacket) {

	if(pClientConn->ubType == CLIENT_TYPE_UNKNOWN && pPacket->sHead.ubType != PACKET_HELLO) {
		logError("Client 0x%p doesn't want to ID itself\n", pClientConn);
		netServerRmClient(pClientConn);
		return;
	}

	switch(pPacket->sHead.ubType) {
		case PACKET_HELLO:
			serverProcessHello(pClientConn, pPacket);
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

void serverProcessHello(tNetConn *pClient, tPacket *pPacket) {
	tNetServer *pServer;
	UBYTE ubNewClientType;

	// Sanity check
	ubNewClientType = pPacket->sHello.ubClientType;
	pServer = &pClient->pClientServer->sServer;
	if(ubNewClientType >= CLIENT_TYPES) {
		logError(
			"Client 0x%p attempts to set type to %hu",
			ubNewClientType
		);
		netServerRmClient(pClient);
    return;
	}
	if(pClient->ubType != CLIENT_TYPE_UNKNOWN) {
		logError(
			"Client 0x%p type change attempt: '%s' (%hu) -> '%s' (%hu)",
			pClient,
			g_szClientTypes[pClient->ubType], pClient->ubType,
			g_szClientTypes[ubNewClientType], ubNewClientType
		);
		netServerRmClient(pClient);
		return;
	}

	uv_mutex_lock(&pServer->sListMutex);
	pClient->ubType = ubNewClientType;
	uv_mutex_unlock(&pServer->sListMutex);
	logWrite(
		"Client 0x%p identified as '%s' (%hu)",
		pClient, g_szClientTypes[pClient->ubType], pClient->ubType
	);
}
