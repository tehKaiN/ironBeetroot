#ifndef GUARD_PROCESS_H
#define GUARD_PROCESS_H

#include "../common/net/net.h"

void processClient(tNetServer *pServer, tNetConn *pClient, tPacket *pPacket);


#endif // GUARD_PROCESS_H
