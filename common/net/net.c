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

	// TODO(#2): add keyboard event handler - A for about, Q for quit, R - reset
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
	logSuccess("End");
}

void netDestroy(void) {
	tListNode *pNode;

	pNode = g_sNetManager.pClientServerList->pHead;
	while(pNode) {
		netDestroyClientServer((tNetClientServer*)pNode->pData);
		pNode = pNode->pNext;
	}

	// TODO(#2): Stop after all client/servers are dead
	if(!g_sNetManager.ubIsRunning) {
		uv_loop_close(g_sNetManager.pLoop);
		g_sNetManager.ubIsRunning = 0;
		logWarning("UV loop stopped");
	}

	listDestroy(g_sNetManager.pClientServerList);
	memFree(g_sNetManager.pLoop);
}

void netOnRead(
	uv_stream_t *pClientStream, ssize_t lDataLength, const uv_buf_t *pBuf
) {
	tPacket sPacket;
	ULONG ulBufPos;
	tNetConn *pConn;
	tNetClientServer *pClientServer;
	UBYTE ubPacketOK;

	// TODO(#6): Client crashes here if server shuts down

	// Check connection validity
	pConn = (tNetConn*)(pClientStream->data);
	if(!pConn) {
		logError("Packet from null connection");
		uv_close((uv_handle_t*) pClientStream, 0);
		return;
	}
	if(!pConn->ubActive) {
			logError("Packet from inactive connection");
			uv_close((uv_handle_t*) pClientStream, 0);
			return;
	}

	// Check for read errors
	pClientServer = pConn->pClientServer;
	if(lDataLength < 0) {
		if(pClientServer->ubType == NET_CLIENT) {
			logError("Connection to server lost");
			netClientReconnect((tNetClient*)pClientServer, lDataLength);
		}
		else {
			logError("Connection to client lost");
      netServerRmClient(pConn);
		}
		return;
	}

  // Is there anything to read?
  if(!lDataLength) {
		logWarning("Empty buffer");
		return;
  }

	if(pClientServer->ubType == NET_SERVER)
		netUpdateConnTime(pConn);

	// Process packets
	// TODO(#1): Make following as cb (protocol-specific code)
	ulBufPos = 0;
	ubPacketOK = 0;
	do {
		ubPacketOK = netGetPacket(&sPacket, pBuf, &ulBufPos, lDataLength);
		if(ubPacketOK)
			pClientServer->pPacketProcess(pClientServer, pConn, &sPacket);
	} while(ubPacketOK && ulBufPos < lDataLength);

	if(!ubPacketOK) {
		logError("Packets are suspicious - abort");
		if(pClientServer->ubType == NET_CLIENT)
			uv_close((uv_handle_t*)&pClientServer->sTCP, 0);
		else
			netServerRmClient(pConn);
	}

	if(pBuf->base)
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

  pHead = (tPacketHead*)&pBuf->base[*pBufPos];

	// Does packet exceed max packet size?
	if(pHead->ubPacketLength > sizeof(tPacket)) {
		logWarning("Packet longer than expected");
		return 0;
	}

  // Does packet exceed buffer size bounds?
  if(pHead->ubPacketLength > lDataLength - *pBufPos) {
		logError(
			"Buffer overflow attempt! Buffer: %lu, packet: %lu+%lu",
			pHead->ubPacketLength,
			*pBufPos,
			pBuf->len
		);
		return 0;
  }

  // Fill packet struct
  memcpy(pPacket, &pBuf->base[*pBufPos], pHead->ubPacketLength);
  if(pHead->ubPacketLength < sizeof(tPacket))
		memset(
			((UBYTE*)pPacket)+pHead->ubPacketLength, 0,
			sizeof(tPacket)-pHead->ubPacketLength
		);

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

void netSend(tNetConn *pConn, tPacket *pPacket, uv_write_cb pOnWrite) {
	uv_buf_t sBuf;
	uv_write_t *pWriteRequest;

	pWriteRequest = memAlloc(sizeof(uv_write_t));
	sBuf.base = (char *)pPacket;
	sBuf.len = pPacket->sHead.ubPacketLength;

	uv_write(pWriteRequest, pConn->pStream, &sBuf, 1, pOnWrite);
}

void netReadOnWrite(uv_write_t* pWriteRequest, LONG lStatus) {
  if (lStatus < 0) {
    logError("UV: %s\n", uv_strerror(lStatus));
    // TODO (#1): reconnect?
    return;
  }
	uv_read_start(pWriteRequest->handle, netAllocBfr, netOnRead);
}

void netNopOnWrite(uv_write_t* pWriteRequest, LONG lStatus) {
  if (lStatus < 0) {
    logError("UV: %s\n", uv_strerror(lStatus));
    // TODO (#1): reconnect?
    return;
  }
	memFree(pWriteRequest);
}

void netAllocBfr(uv_handle_t *pHandle, size_t ulSuggestedSize, uv_buf_t *pBuf) {
	 *pBuf = uv_buf_init(memAlloc(ulSuggestedSize), ulSuggestedSize);
}
