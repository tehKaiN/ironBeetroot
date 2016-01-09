#ifndef GUARD_RAMIE_RAMIE_H
#define GUARD_RAMIE_RAMIE_H

void ramieProcessPacket(
	IN tNetClientServer *pClientServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void ramieOnConnect(
	IN tNetClient *pClient
);

#endif // GUARD_RAMIE_RAMIE_H

