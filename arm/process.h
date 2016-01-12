#ifndef GUARD_ARM_PROCESS_H
#define GUARD_ARM_PROCESS_H

#include "../common/types.h"
#include "../common/net/net.h"

void processPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

#endif // GUARD_ARM_PROCESS_H

