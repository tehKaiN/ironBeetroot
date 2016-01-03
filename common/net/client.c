#include <stdio.h>
#include "client.h"
#include "../log.h"

void netClientCreate(char *szIP, UWORD uwPort, fnPacketProcess pPacketProcess) {
	tListNode *pNode;
	tNetClientServer *pClientServer;
	tNetClient *pClient;

	if(strlen(szIP) > 11) {
		logError("Suspicious IP: %s", szIP);
		return;
	}

	// Setup net list node
  pNode = listAddTail(g_sNetManager.pClientServerList);
  pClientServer = (tNetClientServer*)pNode->pData;
  pClientServer->ubType = NET_CLIENT;
  pClient = &pClientServer->sClient;

	// Setup client struct
	pClient->ubConnectState = CONNECTSTATE_NEW;
	strcpy(pClient->szIP, szIP);
	pClient->uwPort = uwPort;
	pClientServer->pPacketProcess = pPacketProcess;

	uv_tcp_init(g_sNetManager.pLoop, &pClientServer->sTCP);
	uv_ip4_addr(szIP, uwPort, &pClient->sSrvAddr);
	netClientReconnect(pClient, 0);

	++g_sNetManager.ubClientCount;
}

void netClientDestroy(tNetClient *pClient) {

	// TODO: Send disconnect packet to server

	logSuccess("Destroyed client %p", pClient);
}

void netClientReconnect(tNetClient *pClient, LONG lErrorCode) {
	// Display error only after connection breakup
	switch(pClient->ubConnectState) {
		case CONNECTSTATE_LOST:
			logError(
				"Connection to %s@%u lost: %s",
				pClient->szIP, pClient->uwPort, uv_strerror(lErrorCode)
			);
			logWrite("Retrying...");
			break;
		case CONNECTSTATE_NEW:
      logWrite(
				"Attempting to connect: %s@%u...",
				pClient->szIP, pClient->uwPort
			);
			break;
		case CONNECTSTATE_OK:
			logWarning("CONNECTSTATE_OK");
			break;
		case CONNECTSTATE_RETRY:
			putc('.', stdout);
			break;
		default:
			logError("Unknown connectstate: %u", pClient->ubConnectState);
	}

	pClient->ubConnectState = CONNECTSTATE_RETRY;

	// Reconnect
	uv_tcp_connect(
		&pClient->sConn, &((tNetClientServer*)pClient)->sTCP,
		(const struct sockaddr *)&pClient->sSrvAddr,
		netClientOnConnect
	);
}

void netClientOnConnect(uv_connect_t* pConn, LONG lStatus) {
	tNetClient *pClient;
	tPacket sPacket;
  pClient = netClientGetByConnection(pConn);

	if(!pClient) {
		printf("\n");
		logError("Unknown connection response");
		return;
	}
	if(lStatus < 0) {
		netClientReconnect(pClient, lStatus);
		return;
	}
	printf("\n");

	pClient->ubConnectState = CONNECTSTATE_OK;
	logSuccess("Connected, sending client type: %u", CLIENT_TYPE_NADANIE);

//	stream = connection->handle;

	// Send ID packet
	packetMakeHello(&sPacket, CLIENT_TYPE_NADANIE);
	netClientSend(&sPacket, clientReadResponse);
}

tNetClient *netClientGetByConnection(uv_connect_t *pConn) {
	tListNode *pNode;
	tNetClient *pClient;

	uv_mutex_lock(&g_sNetManager.pClientServerList->sMutex);
	pNode = g_sNetManager.pClientServerList->pHead;
	while(pNode) {
		pClient = (tNetClient*)pNode->pData;
		if(&pClient->sConn == pConn) {
			uv_mutex_unlock(&g_sNetManager.pClientServerList->sMutex);
			return pClient;
		}
		pNode = pNode->pNext;
	}
	uv_mutex_unlock(&g_sNetManager.pClientServerList->sMutex);
	return 0;
}
