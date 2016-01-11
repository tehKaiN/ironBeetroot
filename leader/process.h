#ifndef GUARD_LEADER_PROCESS_H
#define GUARD_LEADER_PROCESS_H

#include "../common/types.h"
#include "../common/net/net.h"

void processServerPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pClientConn,
	IN tPacket *pPacket
);

void processSetType(
	IN tNetConn *pClientConn,
	IN tPacketSetType *pPacket
);

void processClientOnConnect(
	IN tNetClient *pClient
);

void processClientPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pSrvConn,
	IN tPacket *pPacket
);

void processSetTypeResponse(
	IN tPacketSetTypeResponse *pResponse
);

void processPlatformListResponse(
	IN tPacketPlatformList *pPacket
);

void processPackageListResponse(
	IN tPacketPackageList *pPacket
);

#endif // GUARD_LEADER_PROCESS_H

