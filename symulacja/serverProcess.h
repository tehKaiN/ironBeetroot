#ifndef GUARD_SERVERPROCESS_H
#define GUARD_SERVERPROCESS_H

#include "../common/types.h"
#include "../common/net/net.h"

void serverProcessProtocol(
	IN tNetServer *pServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void serverProcessHello(
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

#endif // GUARD_SERVERPROCESS_H
