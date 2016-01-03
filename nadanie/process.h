#ifndef GUARD_PROCESS_H
#define GUARD_PROCESS_H

#include "../common/net/net.h"

void processClient(
	IN tNetClientServer *pClientServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);


#endif // GUARD_PROCESS_H
