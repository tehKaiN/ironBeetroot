#include <stdio.h>

#include "../common/types.h"
#include "../common/mem.h"
#include "../common/log.h"
#include "client.h"

tClientConn g_sClientConn;

void onConnect(uv_connect_t* connection, LONG lStatus) {
	if(lStatus < 0)
		clientReconnect(lStatus);
	else {
		printf("\n");
		g_sClientConn.ubConnectState = CONNECTSTATE_OK;
		logSuccess("Connected\n");

//		stream = connection->handle;

		// Call custom handler fn
		g_sClientConn.pOnConnect();

		// Set keepalive timer
	}
}

void clientRun(char *szIP, UWORD uwPort, fnOnConnect pOnConnect) {

	g_sClientConn.pLoop = memAlloc(sizeof(uv_loop_t));
	g_sClientConn.pOnConnect = pOnConnect;
	g_sClientConn.ubConnectState = CONNECTSTATE_NEW;

	uv_loop_init(g_sClientConn.pLoop);
	uv_tcp_init(g_sClientConn.pLoop, &g_sClientConn.sTCP);
	uv_ip4_addr(szIP, uwPort, &g_sClientConn.sServerAddr);
	clientReconnect(0);

  uv_run(g_sClientConn.pLoop, UV_RUN_DEFAULT);
  uv_loop_close(g_sClientConn.pLoop);

  memFree(g_sClientConn.pLoop);
}

void clientReconnect(LONG lErrorCode) {
	char szIPBfr[16];
	UWORD uwPort;

	// Display erorr only after connection breakup
	switch(g_sClientConn.ubConnectState) {
		case CONNECTSTATE_LOST:
			logError("Connection fail: %s\n", uv_strerror(lErrorCode));
			logWrite("Retrying...");
			break;
		case CONNECTSTATE_NEW:
			uv_ip4_name(&g_sClientConn.sServerAddr, szIPBfr, 16);
			uwPort = g_sClientConn.sServerAddr.sin_port;
			uwPort = ((uwPort&0xFF00) >> 8) | ((uwPort & 0x00FF) << 8);
      logWrite("Attempting to connect: %s@%u...", szIPBfr, uwPort);
			break;
		case CONNECTSTATE_OK:
			logError("[clientReconnect] CONNECTSTATE_OK\n");
			break;
		case CONNECTSTATE_RETRY:
			printf(".");
			break;
	}

	g_sClientConn.ubConnectState = CONNECTSTATE_RETRY;

	// Reconnect
	uv_tcp_connect(
		&g_sClientConn.sConn, &g_sClientConn.sTCP,
		(const struct sockaddr *)&g_sClientConn.sServerAddr,
		onConnect
	);
}

void clientSend(tPacket *pPacket) {
	uv_buf_t sBuf;
	uv_write_t sWriteRequest;

	sBuf.base = (char *)pPacket;
	sBuf.len = pPacket->sHead.ubPacketLength;

	uv_write(&sWriteRequest, g_sClientConn.sConn.handle, &sBuf, 1, clientOnWrite);
}

void clientOnWrite(uv_write_t* pWriteRequest, LONG lStatus) {
  if (lStatus < 0) {
    logError("[clientSend] Write error: %s\n", uv_strerror(lStatus));
    return;
  }
}
