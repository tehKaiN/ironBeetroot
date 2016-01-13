#ifndef GUARD_SHOW_PROCESS_H
#define GUARD_SHOW_PROCESS_H

#include "../common/types.h"
#include "../common/net/net.h"
#include "../common/packet.h"

void processHallPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pConn,
	IN tPacket *pPacket
);

void processLeaderPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pConn,
	IN tPacket *pPacket
);

void processSetTypeResponse(
	IN tPacketSetTypeResponse *pResponse
);

#endif // GUARD_SHOW_PROCESS_H

