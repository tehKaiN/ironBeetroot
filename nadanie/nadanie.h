#ifndef GUARD_NADANIE_NADANIE_H
#define GUARD_NADANIE_NADANIE_H

void nadanieProcessPacket(
	IN tNetClientServer *pClientServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void nadanieOnConnect(
	IN tNetClient *pClient
);

#endif // GUARD_NADANIE_NADANIE_H
