#ifndef GUARD_COMMON_NET_SERVER_H
#define GUARD_COMMON_NET_SERVER_H

#include "../types.h"
#include "net.h"

void netServerCreate(
	IN UBYTE ubMaxClients,
	IN UWORD uwPort,
	IN fnPacketProcess *pPacketProcess,
	IN UBYTE ubBacklogLength
);

void netServerDestroy(
	IN tNetServer *pServer
);

void netServerOnConnect(
	IN uv_stream_t *pServerStream,
	IN LONG lStatus
);

tNetConn *netServerAcceptClient(
	IN tNetServer *pServer
);

UBYTE netServerGetFreeClientIdx(
	IN tNetServer *pServer,
	IN UBYTE *pIdx
);

void netServerRmClient(
	IN tNetConn *pClient
);

void netServerRmClientByIdx(
	IN tNetServer *pServer,
	IN UBYTE ubIdx
);

void netServerRmAll(
	IN tNetServer *pServer
);

void netServerKeepAlive(
	IN tNetServer *pServer
);

#endif // GUARD_COMMON_NET_SERVER_H

