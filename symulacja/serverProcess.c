#include "serverProcess.h"
#include "../common/log.h"
#include "../common/packet.h"
#include "server.h"

void serverProcessProtocol(tClient *pClient, tPacket *pPacket) {
	if(pClient->ubType == CLIENT_TYPE_UNKNOWN && pPacket->sHead.ubType != PACKET_HELLO) {
		logError("Client %p doesn't want to ID itself\n", pClient);
		// TODO: close client
		return;
	}
	switch(pPacket->sHead.ubType) {
		case PACKET_HELLO:
			if(pClient->ubType != CLIENT_TYPE_UNKNOWN) {
				logError(
					"[serverProcessProtocol] Client %p type change attempt: %hu -> %hu\n",
					pClient, pClient->ubType, pPacket->sHello.ubClientType
				);
				// TODO: close client
				return;
			}
			uv_mutex_lock(&g_sServer.sMutex);
			pClient->ubType = pPacket->sHello.ubClientType;
			uv_mutex_unlock(&g_sServer.sMutex);
			logWrite();
			break;
		default:
			logWarning("[serverProcessProtocol] Unknown packet type: %hu\n", pPacket->sHead.ubType);
      logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}
	serverUpdateClientTime(pClient);
}
