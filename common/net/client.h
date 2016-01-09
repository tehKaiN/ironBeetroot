#ifndef GUARD_COMMON_NET_CLIENT_H
#define GUARD_COMMON_NET_CLIENT_H

#include <uv.h>
#include "../types.h"
#include "net.h"

tNetClient *netClientCreate(
	IN char *szIP,
	IN UWORD uwPort,
	IN fnOnConnect pOnConnect,
	IN fnPacketProcess pPacketProcess
);

void netClientDestroy(
	IN tNetClient *pClient
);

void netClientReconnect(
	IN tNetClient *pClient,
	IN LONG lErrorCode
);

void netClientOnConnect(
	IN uv_connect_t* sConn,
	IN LONG lStatus
);

void netClientOnWrite(
	IN uv_write_t* pWriteRequest,
	IN LONG lStatus
);

tNetClient *netClientGetByUvConn(
	IN uv_connect_t *pConn
);

#endif // GUARD_COMMON_NET_CLIENT_H
