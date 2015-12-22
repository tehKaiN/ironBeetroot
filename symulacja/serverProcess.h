#ifndef GUARD_SERVERPROCESS_H
#define GUARD_SERVERPROCESS_H

#include "../common/types.h"
#include "server.h"

void serverProcessProtocol(
	IN tClient *pClient,
	IN tPacket *pPacket
);

#endif // GUARD_SERVERPROCESS_H
