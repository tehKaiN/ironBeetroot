#include "net.h"
#include "../log.h"
#include "../mem.h"
#include "server.h" // netServerDestroy
#include "client.h" // netClientDestroy

tNetManager g_sNetManager;

void netCreate(void) {
	g_sNetManager.pClientServerList = listCreate(sizeof(tNetClientServer));
	g_sNetManager.ubIsRunning = 0;
	g_sNetManager.ubClientCount = 0;
	g_sNetManager.ubServerCount = 0;

	// New uv event loop
	g_sNetManager.pLoop = memAlloc(sizeof(uv_loop_t));
	uv_loop_init(g_sNetManager.pLoop);

	// TODO: add keyboard event handler - A for about, Q for quit
}

void netRun(void) {
	if(g_sNetManager.ubIsRunning) {
    logWarning("Already running!");
		return;
	}

	logSuccess(
		"Start, clients: %hu, servers: %hu",
		g_sNetManager.ubClientCount,
		g_sNetManager.ubServerCount
	);
	g_sNetManager.ubIsRunning = 1;
	uv_run(g_sNetManager.pLoop, UV_RUN_DEFAULT);
	logWarning("UV loop end");
	uv_loop_close(g_sNetManager.pLoop);
	g_sNetManager.ubIsRunning = 0;
}

void netDestroy(void) {
	tListNode *pNode;

	if(!g_sNetManager.ubIsRunning) {
		// TODO: Stop uv loop?
	}

	pNode = g_sNetManager.pClientServerList->pHead;
	while(pNode) {
		netDestroyClientServer((tNetClientServer*)pNode->pData);
		pNode = pNode->pNext;
	}

	listDestroy(g_sNetManager.pClientServerList);

	// TODO: UV cleanup
}

void netOnRead(uv_stream_t *pClientStream, ssize_t lDataLength, const uv_buf_t *pBuf) {
	tPacket sPacket;
	ULONG ulBufPos;
	tNetConn *pClient;
	tNetClientServer *pClientServer;

	pClient = (tNetConn*)(pClientStream->data);
	pClientServer = pClient->pClientServer;
	ulBufPos = 0;
	// Is packet coming from active client from list?
	if(!pClient) {
		logWarning("Packet from unknown client");
		uv_close((uv_handle_t*) pClientStream, 0);
		return;
	}
	if(!pClient->ubActive) {
			logWarning("Packet from inactive client");
			uv_close((uv_handle_t*) pClientStream, 0);
			return;
	}
	netUpdateConnTime(pClient);

	// Are there packets in buffer?
	do {
		// Get next packet from buffer
		if(netGetPacket(&sPacket, pBuf, &ulBufPos, lDataLength)) {
			// TODO: Process packet
			pClientServer->pPacketProcess(&pClientServer->sServer, pClient, &sPacket);
		}
	} while(ulBufPos < lDataLength);

	memFree(pBuf->base);
}

void netDestroyClientServer(tNetClientServer *pClientServer) {

  switch(pClientServer->ubType) {
		case NET_CLIENT:
			netClientDestroy(&pClientServer->sClient);
			break;
		case NET_SERVER:
			netServerDestroy(&pClientServer->sServer);
			break;
		case NET_UNKNOWN:
			logError(
				"Unknown client/server type: %hu",
				pClientServer->ubType
			);
			return;
  }
}

UBYTE netGetPacket(tPacket *pPacket, const uv_buf_t *pBuf, ULONG *pBufPos, LONG lDataLength) {
	tPacketHead *pHead;

	// Was read successful?
  if(lDataLength < 0) {
		logError("Read error: %s", uv_strerror(lDataLength));
		return 0;
  }

  // Is there anything to read?
  if(lDataLength == 0) {
		logWarning("Empty buffer");
		return 0;
  }

  pHead = (tPacketHead*)&pBuf->base[*pBufPos];

	// Does packet exceed max packet size?
	if(pHead->ubPacketLength > sizeof(tPacket)) {
		logWarning("Packet longer than expected");
		// TODO: close client
		return 0;
	}

  // Does packet exceed buffer size bounds?
  if(pHead->ubPacketLength > *pBufPos + pBuf->len) {
		logError(
			"Buffer overflow attempt! Buffer: %lu, packet: %lu+%lu",
			pHead->ubPacketLength,
			*pBufPos,
			pBuf->len
		);
		// TODO: close client
		return 0;
  }

  // Fill packet struct
  memcpy(pPacket, &pBuf->base[*pBufPos], pHead->ubPacketLength);
  if(pHead->ubPacketLength < sizeof(tPacket))
		memset(((UBYTE*)pPacket)+pHead->ubPacketLength, 0, sizeof(tPacket)-pHead->ubPacketLength);

  *pBufPos += pHead->ubPacketLength;
  return 1;
}

void netUpdateConnTime(tNetConn *pConn) {
	tNetServer *pServer;

	pServer = &pConn->pClientServer->sServer;
	uv_mutex_lock(&pServer->sListMutex);
  time(&pConn->llLastPacketTime);
	uv_mutex_unlock(&pServer->sListMutex);
}
