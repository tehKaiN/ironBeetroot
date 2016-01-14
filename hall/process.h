#ifndef GUARD_PROCESS_H
#define GUARD_PROCESS_H

#include "../common/types.h"
#include "../common/net/net.h"

void processProtocol(
	IN tNetServer *pServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void processSetType(
	IN tNetConn *pClient,
	IN tPacketSetType *pPacket
);

void processPlatformInfo(
	IN tNetConn *pClientConn
);

void processPlatformInfoGiveTake(
	IN tNetConn *pClientConn
);

void processUpdatePlatforms(
	IN tNetConn *pClientConn,
	IN tPacketUpdatePlatforms *pRequest
);

void processPlatformList(
	IN tNetConn *pClientConn
);

void processPackageList(
	IN tNetConn *pClientConn
);

UBYTE _hallCheckClient(
	IN tNetConn *pClientConn,
	IN UBYTE ubType,
	IN const char *szFnName
);

#define hallCheckClient(pClient, ubType) \
	_hallCheckClient(pClient, ubType, __func__)

#endif // GUARD_PROCESS_H
