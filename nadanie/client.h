#ifndef GUARD_CLIENT_H
#define GUARD_CLIENT_H

#include <uv.h>
#include "../common/packet.h"

#define CONNECTSTATE_LOST 0
#define CONNECTSTATE_NEW 1
#define CONNECTSTATE_OK 2
#define CONNECTSTATE_RETRY 3

typedef void (*fnOnConnect)(void);

typedef struct _tClient{
	uv_tcp_t sTCP;
	uv_connect_t sConn;
	uv_loop_t *pLoop;
	fnOnConnect pOnConnect;
	struct sockaddr_in sServerAddr;
	UBYTE ubConnectState;
} tClientConn;

void clientRun(
	IN char *szIP,
	IN UWORD uwPort,
	IN fnOnConnect pOnConnect
);

void clientReconnect(LONG lErrorCode);

void clientSend(
	IN tPacket *pPacket
);

void clientOnWrite(
	IN uv_write_t* pWriteRequest,
	IN LONG lStatus
);

#endif // GUARD_CLIENT_H

