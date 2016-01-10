#ifndef GUARD_CUSTOMERP_ROCESS_H
#define GUARD_CUSTOMERP_ROCESS_H

#include "../common/types.h"
#include "../common/net/net.h"

void processPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pConn,
	IN tPacket *pPacket
);

void processSetTypeResponse(
	IN tPacketSetTypeResponse *pResponse
);

void processPlatformInfoResponse(
	IN tPacketPlatformInfo *pInfo
);

void processUpdatePlatformsResponse(
	IN tPacketUpdatePlatformsResponse *pResponse
);

#endif // GUARD_CUSTOMERP_ROCESS_H

