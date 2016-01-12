#ifndef GUARD_ARM_CMD_H
#define GUARD_ARM_CMD_H

#include "../common/types.h"
#include "../common/packet.h"

UBYTE cmdIsDone(void);

void cmdMakeStopPacket(
	OUT tPacketActuators *pActuators
);

void cmdStopActuators(void);

UBYTE cmdGoNext(void);

void cmdProcessCurr(void);

#endif // GUARD_ARM_CMD_H
