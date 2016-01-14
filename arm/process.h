#ifndef GUARD_ARM_PROCESS_H
#define GUARD_ARM_PROCESS_H

#include "../common/types.h"
#include "../common/net/net.h"

void processHallPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void processLeaderPacket(
	IN tNetClientServer *pCS,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void processSetTypeResponse(
	IN tPacketSetTypeResponse *pResponse
);

void processSensorInfoResponse(
	IN tPacketSensorInfo *pPacket
);

void processArmPos(void);

void processSetCommands(
	IN tPacketArmCommands *pPacket
);

#endif // GUARD_ARM_PROCESS_H

