#include "serverProcess.h"
#include "../common/mem.h"
#include "../common/log.h"
#include "../common/packet.h"
#include "../common/net.h"

void serverProcessProtocol(tNetServer *pServer, tNetConn *pClient, tPacket *pPacket) {

	if(pClient->ubType == CLIENT_TYPE_UNKNOWN && pPacket->sHead.ubType != PACKET_HELLO) {
		logError("Client %p doesn't want to ID itself\n", pClient);
		netServerRmClient(pClient);
		return;
	}

	switch(pPacket->sHead.ubType) {
		case PACKET_HELLO:
			serverProcessHello(pClient, pPacket);
			break;
		default:
			logWarning("[serverProcessProtocol] Unknown packet type: %hu\n",
				pPacket->sHead.ubType
			);
      logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}

	if(pClient->ubActive) {
		uv_read_start((uv_stream_t*)pClient->pTCP, memAllocUvBfr, netOnRead);
		netServerUpdateClientTime(pClient);
	}
}

void serverProcessHello(tNetConn *pClient, tPacket *pPacket) {
	tNetServer *pServer;

	pServer = &pClient->pClientServer->sServer;
	if(pClient->ubType != CLIENT_TYPE_UNKNOWN) {
		logError(
			"[serverProcessProtocol] Client %p type change attempt: %hu -> %hu\n",
			pClient, pClient->ubType, pPacket->sHello.ubClientType
		);
		netServerRmClient(pClient);
		return;
	}
	uv_mutex_lock(&pServer->sListMutex);
	pClient->ubType = pPacket->sHello.ubClientType;
	uv_mutex_unlock(&pServer->sListMutex);
	logWrite("Client %p identified as %s", pClient, "TODO"); // TODO: display type
}
