#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/net/client.h"
#include "../common/packet.h"
#include "nadanie.h"
#include "config.h"
#include "process.h"

int main(void) {
	logCreate("nadanie.log");
	memCreate();
	netCreate();

	netClientCreate("127.0.0.1", 888, nadanieOnConnect, nadanieProcessPacket);
	netRun();

	netDestroy();
	memDestroy();
	logDestroy();
	return 0;
}

void nadanieOnConnect(tNetClient *pClient) {
	tPacket sPacket;

	// Send ID packet
	logWrite(
		"Sending client type: '%s\' (%u)",
		g_szClientTypes[CLIENT_TYPE_NADANIE], CLIENT_TYPE_NADANIE
	);

	packetMakeSetType(&sPacket, CLIENT_TYPE_NADANIE);
	netSend(&pClient->sSrvConn, &sPacket, netReadAfterWrite);
}

void nadanieProcessPacket(tNetClientServer *pServer, tNetConn *pClient, tPacket *pPacket) {

	switch(pPacket->sHead.ubType) {

	}
}






