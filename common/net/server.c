#include "server.h"
#include "../mem.h"
#include "../log.h"
#include <uv.h>

void netServerCreate(UBYTE ubMaxClients, UWORD uwPort, fnPacketProcess *pPacketProcess, UBYTE ubBacklogLength) {
	tListNode *pNode;                /// Net list node
	tNetClientServer *pClientServer; /// Server container struct
	tNetServer *pServer;             /// Server struct
	LONG lError;                     /// UV error code
	struct sockaddr_in sListenAddr;  /// Listen address config


	// Setup net list node
  pNode = listAddTail(g_sNetManager.pClientServerList);
  pClientServer = (tNetClientServer*)pNode->pData;
  pClientServer->ubType = NET_SERVER;
  pServer = &pClientServer->sServer;

	// Setup server struct
	pServer->ubMaxClients = ubMaxClients;
	pServer->pClients = memAlloc(ubMaxClients * sizeof(tNetConn));
	memset(pServer->pClients, 0, sizeof(tNetConn) * ubMaxClients);
	uv_mutex_init(&pServer->sListMutex);
	pClientServer->pPacketProcess = pPacketProcess;

	// New listen connection - 0.0.0.0 is any IP address

	uv_tcp_init(g_sNetManager.pLoop, &pClientServer->sTCP);
	pClientServer->sTCP.data = pClientServer;
	uv_ip4_addr("0.0.0.0", uwPort, &sListenAddr);
  uv_tcp_bind(&pClientServer->sTCP, (const struct sockaddr*)&sListenAddr, 0);


	lError = uv_listen((uv_stream_t*)(&pClientServer->sTCP), ubBacklogLength, netServerOnConnect);
	if(lError) {
		logError("Listen fail: %s", uv_strerror(lError));
		return;
	}
	logSuccess("Listening @port: %u", uwPort);
	++g_sNetManager.ubServerCount;
}

void netServerOnConnect(uv_stream_t *pServerStream, LONG lStatus) {
	LONG lError;
	tNetServer *pServer;
	tNetConn *pClient;

	pServer = &((tNetClientServer*)(pServerStream->data))->sServer;

	if(lStatus < 0) {
		logError("Connect: %s", uv_strerror(lStatus));
		return;
	}
	logSuccess("Connected: %s@%u", "?.?.?.?", 0); // TODO(#3): display IP, port

	/// Add to client list
	pClient = netServerAcceptClient(pServer);

	/// Await ID packet
	lError = uv_read_start(pClient->pStream, netAllocBfr, netOnRead);
	if(lError < 0) {
		logError("Read: %s", uv_strerror(lError));
		netServerRmClient(pClient);
		return;
	}
}

tNetConn *netServerAcceptClient(tNetServer *pServer) {
	LONG lError;
	UBYTE ubIdx;
	uv_tcp_t *pClientTCP;
	tNetConn *pClientConn;
	tNetClientServer *pClientServer;

	pClientServer = (tNetClientServer*)pServer;
	if(!netServerGetFreeClientIdx(pServer, &ubIdx)) {
		logError("No more slots in client list");
		return 0;
	}

	uv_mutex_lock(&pServer->sListMutex);

	/// Accept client
	pClientTCP = memAlloc(sizeof(uv_tcp_t));
	uv_tcp_init(g_sNetManager.pLoop, pClientTCP);
	lError = uv_accept((uv_stream_t*)&pClientServer->sTCP, (uv_stream_t*)pClientTCP);
	if(lError < 0) {
		logError("Accept: %s", uv_strerror(lError));
		uv_close((uv_handle_t*)pClientTCP, 0);
		memFree(pClientTCP);
		uv_mutex_unlock(&pServer->sListMutex);
		return 0;
	}

	/// Fill client connection struct
	pClientConn = &pServer->pClients[ubIdx];
	pClientConn->ubActive = 1;
	pClientConn->pStream = (uv_stream_t*)pClientTCP;
	pClientConn->ubType = CLIENT_TYPE_UNKNOWN;
	pClientConn->pClientServer = pClientServer;
	time(&pClientConn->llLastPacketTime);
	pClientTCP->data = pClientConn;

	logSuccess("Client accepted");
	// TODO(#3): Display IP, port and assigned number

	uv_mutex_unlock(&pServer->sListMutex);
	return pClientConn;
}

void netServerDestroy(tNetServer *pServer) {

	// TODO(#4): Send disconnect packet to connected clients
	// TODO(#4): Close all clients

	// TODO(#4): After: close server

	// Cleanup after done
	// TODO(#4): After
	uv_mutex_destroy(&pServer->sListMutex);
  memFree(pServer->pClients);
}

UBYTE netServerGetFreeClientIdx(tNetServer *pServer, UBYTE *pIdx) {
	UBYTE i;

	uv_mutex_lock(&pServer->sListMutex);
  for(i = 0; i != pServer->ubMaxClients; ++i)
		if(!pServer->pClients[i].ubActive) {
			*pIdx = i;
			uv_mutex_unlock(&pServer->sListMutex);
			return 1;
		}
	uv_mutex_unlock(&pServer->sListMutex);
	return 0;
}

void netServerRmClient(tNetConn *pClient) {
	tNetServer *pServer;

	pServer = &pClient->pClientServer->sServer;
	uv_mutex_lock(&pServer->sListMutex);
	if(!pClient->ubActive) {
		uv_mutex_unlock(&pServer->sListMutex);
		logWarning("Client 0x%p is already inactive", pClient);
		return;
	}
	uv_close((uv_handle_t*)pClient->pStream, 0);
	pClient->ubActive = 0;
	uv_mutex_unlock(&pServer->sListMutex);
	logWrite("Client disconnected: 0x%p", pClient);
}

void netServerRmClientByIdx(tNetServer *pServer, UBYTE ubIdx) {
	if(ubIdx >= pServer->ubMaxClients) {
		logWarning("Client idx too high: %hu, max: %hu", ubIdx, pServer->ubMaxClients-1);
		return;
	}
	logWrite("Disconnecting client 0x%p @idx: %hu...", pServer->pClients[ubIdx], ubIdx);
	netServerRmClient(&pServer->pClients[ubIdx]);
}

void netServerRmAll(tNetServer *pServer) {
	UBYTE i;
	logWrite("Disconnecting all clients...");
	for(i = pServer->ubMaxClients; i--;) {
		if(pServer->pClients[i].ubActive)
			netServerRmClient(&pServer->pClients[i]);
	}
	logWrite("All clients disconnected");
}

void netServerKeepAlive(tNetServer *pServer) {
  static UBYTE ubClientIdx = 0;
  tNetConn *pClient;
  time_t llNow;

	uv_mutex_lock(&pServer->sListMutex);
	time(&llNow);
	pClient = &pServer->pClients[ubClientIdx];
  if(pClient->ubActive && difftime(pClient->llLastPacketTime, llNow) > pServer->uwTimeout) {
		uv_mutex_unlock(&pServer->sListMutex);
		logWarning("Communication lost with client 0x%p", pClient);
		netServerRmClient(pClient);
  }
  uv_mutex_unlock(&pServer->sListMutex);

  ++ubClientIdx;
  if(ubClientIdx >= pServer->ubMaxClients)
		ubClientIdx = 0;
}
