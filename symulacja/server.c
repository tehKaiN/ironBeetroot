#include <time.h>
#include <uv.h>

#include "../common/mem.h"
#include "../common/log.h"

#include "config.h"
#include "server.h"

tServer g_sServer;

void serverRun(UBYTE ubMaxClients, UWORD uwPort, fnPacketProcess *pPacketProcess) {
	LONG lError;
	struct sockaddr_in sListenAddr; /// Listen address config
	uv_tcp_t sTCP;                  /// TCP Handle

	// Setup server struct
	g_sServer.ubMaxClients = ubMaxClients;
	g_sServer.pClientList = memAlloc(ubMaxClients * sizeof(tClient));
	memset(g_sServer.pClientList, 0, sizeof(tClient) * ubMaxClients);
	uv_mutex_init(&g_sServer.sMutex);
	g_sServer.pPacketProcess = pPacketProcess;

	// New uv event loop
	g_sServer.pLoop = memAlloc(sizeof(uv_loop_t));
	uv_loop_init(g_sServer.pLoop);

	// New listen connection - 0.0.0.0 is any IP address
	uv_tcp_init(g_sServer.pLoop, &sTCP);
	uv_ip4_addr("0.0.0.0", uwPort, &sListenAddr);
  uv_tcp_bind(&sTCP, (const struct sockaddr*)&sListenAddr, 0);

	lError = uv_listen((uv_stream_t*)&sTCP, g_sConfig.ubBacklogLength, serverOnConnect);
	if(lError)
		logError("%s", uv_strerror(lError));
	else {
		logSuccess("Listening @port: %u\n", uwPort);
		uv_run(g_sServer.pLoop, UV_RUN_DEFAULT);
		logWarning("Server shutdown\n");
		uv_loop_close(g_sServer.pLoop);
	}

	// Cleanup after done
	uv_mutex_destroy(&g_sServer.sMutex);
	memFree(g_sServer.pClientList);
	memFree(g_sServer.pLoop);
}

void serverOnConnect(uv_stream_t *pServerStream, LONG lStatus) {
	LONG lError;
	tClient *pClient;

	if(lStatus < 0) {
		logError("[serverOnConnect] Connect: %s\n", uv_strerror(lStatus));
		return;
	}
	logSuccess("Connected: ?\n");
	/// TODO: display IP

	/// Add to client list
	pClient = serverAddClient(pServerStream);

	/// Await ID packet
	lError = uv_read_start((uv_stream_t*)pClient->pTCP, serverAllocFn, serverOnRead);
	if(lError < 0) {
		logError("[serverOnConnect] Read: %s\n", uv_strerror(lError));
		serverRmClient(pClient);
		return;
	}
}

void serverOnRead(uv_stream_t *pClientStream, ssize_t lDataLength, const uv_buf_t *pBuf) {

	tPacket sPacket;
	ULONG ulBufPos = 0;
	tClient *pClient;

	// Is packet coming from active client from list?
	pClient = serverGetClientByTCP((uv_tcp_t*)pClientStream); // Rzutowanie legalne?
	if(!pClient) {
		logWarning("[serverOnRead] Packet from unknown client\n");
		uv_close((uv_handle_t*) pClientStream, 0);
		return;
	}
	serverUpdateClientTime(pClient);

	// Are there packets in buffer?
	do {
		// Get next packet from buffer
		if(serverGetPacket(&sPacket, pBuf, &ulBufPos, lDataLength)) {
			// Process packet
			g_sServer.pPacketProcess(pClient, &sPacket);
		}
	} while(ulBufPos < lDataLength);

	memFree(pBuf->base);

  uv_read_start(pClientStream, serverAllocFn, serverOnRead);
}

void serverAllocFn(uv_handle_t *pHandle, size_t uiSuggestedSize, uv_buf_t *pBuf) {
	 *pBuf = uv_buf_init(memAlloc(sizeof(uiSuggestedSize)), uiSuggestedSize);
}

UBYTE serverGetFreeClientIdx(UBYTE *pIdx) {
	UBYTE i;

	uv_mutex_lock(&g_sServer.sMutex);
  for(i = 0; i != g_sServer.ubMaxClients; ++i)
		if(!g_sServer.pClientList[i].ubActive) {
			*pIdx = i;
			uv_mutex_unlock(&g_sServer.sMutex);
			return 1;
		}
	uv_mutex_unlock(&g_sServer.sMutex);
	return 0;
}

tClient *serverAddClient(uv_stream_t *pServerStream) {
	LONG lError;
	UBYTE ubIdx;
	uv_tcp_t *pClientTCP;
	tClient *pClient;

	if(!serverGetFreeClientIdx(&ubIdx)) {
		logError("[serverAddClient] No more slots in client list\n");
		return 0;
	}

	uv_mutex_lock(&g_sServer.sMutex);

	/// Accept client
	pClientTCP = memAlloc(sizeof(uv_tcp_t));
	uv_tcp_init(g_sServer.pLoop, pClientTCP);
	lError = uv_accept(pServerStream, (uv_stream_t*)pClientTCP);
	if(lError < 0) {
		logError("[serverAddClient]Accept: %s\n", uv_strerror(lError));
		uv_close((uv_handle_t*)pClientTCP, 0);
		memFree(pClientTCP);
		uv_mutex_unlock(&g_sServer.sMutex);
		return 0;
	}

	/// Fill client struct
	pClient = &g_sServer.pClientList[ubIdx];
	pClient->ubActive = 1;
	pClient->pTCP = pClientTCP;
	pClient->ubType = CLIENT_TYPE_UNKNOWN;
	time(&pClient->llLastPacketTime);

	logSuccess("Client accepted\n"); // TODO: IP
	uv_mutex_unlock(&g_sServer.sMutex);
	return pClient;
}

tClient *serverGetClientByTCP(uv_tcp_t *pTCP) {
	UBYTE i;

	uv_mutex_lock(&g_sServer.sMutex);
  for(i = 0; i != g_sServer.ubMaxClients; ++i)
		if(g_sServer.pClientList[i].ubActive && g_sServer.pClientList[i].pTCP == pTCP) {
			uv_mutex_unlock(&g_sServer.sMutex);
			return &g_sServer.pClientList[i];
		}
	uv_mutex_unlock(&g_sServer.sMutex);
	return 0;
}

void serverRmClient(tClient *pClient) {
	uv_mutex_lock(&g_sServer.sMutex);
	if(!pClient->ubActive) {
		uv_mutex_unlock(&g_sServer.sMutex);
		logWarning("[serverRmClient] Client %P is already inactive\n", pClient);
		return;
	}
	uv_close((uv_handle_t*)pClient->pTCP, 0);
	pClient->ubActive = 0;
	uv_mutex_unlock(&g_sServer.sMutex);
	logWrite("Client disconnected: %P\n", pClient);
}

void serverRmClientByIdx(UBYTE ubIdx) {
	if(ubIdx >= g_sServer.ubMaxClients) {
		logWarning("[serverRmClientByIdx] Client idx too high: %hu, max: %hu\n", ubIdx, g_sServer.ubMaxClients-1);
		return;
	}
	logWrite("Disconnecting client %P @idx: %hu...\n", g_sServer.pClientList[ubIdx], ubIdx);
	serverRmClient(&g_sServer.pClientList[ubIdx]);
}

void serverRmAll(void) {
	UBYTE i;
	logWrite("Disconnecting all clients...\n");
	for(i = 0; i != g_sServer.ubMaxClients; ++i) {
		if(g_sServer.pClientList[i].ubActive)
			serverRmClient(&g_sServer.pClientList[i]);
	}
	logWrite("All clients disconnected\n");
}

void serverUpdateClientTime(tClient *pClient) {
	uv_mutex_lock(&g_sServer.sMutex);
  time(&pClient->llLastPacketTime);
	uv_mutex_unlock(&g_sServer.sMutex);
}

void serverKeepAlive(void) {
  static UBYTE ubClientIdx = 0;
  tClient *pClient;
  time_t llNow;

	uv_mutex_lock(&g_sServer.sMutex);
	time(&llNow);
	pClient = &g_sServer.pClientList[ubClientIdx];
  if(pClient->ubActive && difftime(pClient->llLastPacketTime, llNow) > g_sConfig.uwServerTimeout) {
		uv_mutex_unlock(&g_sServer.sMutex);
		logWarning("[serverKeepAlive]Communication lost with client %P\n", pClient);
		serverRmClient(pClient);
  }
  uv_mutex_unlock(&g_sServer.sMutex);

  ++ubClientIdx;
  if(ubClientIdx >= g_sServer.ubMaxClients)
		ubClientIdx = 0;
}

UBYTE serverGetPacket(tPacket *pPacket, const uv_buf_t *pBuf, ULONG *pBufPos, LONG lDataLength) {
	tPacketHead *pHead;

	// Was read successful?
  if(lDataLength < 0) {
		logError("[serverGetPacket] Read error: %s\n", uv_strerror(lDataLength));
		return 0;
  }

  // Is there anything to read?
  if(lDataLength == 0) {
		logWarning("[serverGetPacket] Empty buffer\n");
		return 0;
  }

  pHead = (tPacketHead*)&pBuf->base[*pBufPos];

	// Does packet exceed max packet size?
	if(pHead->ubPacketLength > sizeof(tPacket)) {
		logWarning("[serverGetPacket] Packet longer than expected\n");
		// TODO: close client
		return 0;
	}

  // Does packet exceed buffer size bounds?
  if(pHead->ubPacketLength > *pBufPos + pBuf->len) {
		logError(
			"[serverGetPacket] Buffer overflow attempt! Buffer: %lu, packet: %lu+%lu\n",
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
