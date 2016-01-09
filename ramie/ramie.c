#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/net/client.h"
#include "../common/packet.h"
#include "ramie.h"
#include "process.h"

int main(void){
    logCreate("ramie.log");
    memCreate();
    netCreate();

    netClientCreate("127.0.0.1", 888, ramieOnConnect, ramieProcessPacket);
    netRun();

    netDestroy();
    memDestroy();
    logDestroy();
    return 0;
}

void ramieOnConnect(tNetClient *pClient) {
	tPacket sPacket;

	// Send ID packet
	logWrite(
		"Sending client type: '%s\' (%u)",
		g_szClientTypes[CLIENT_TYPE_RAMIE], CLIENT_TYPE_RAMIE
	);

	packetMakeSetType(&sPacket, CLIENT_TYPE_RAMIE);
	netSend(&pClient->sSrvConn, &sPacket, netReadAfterWrite);
}

void ramieProcessPacket(tNetClientServer *pServer, tNetConn *pClient, tPacket *pPacket) {

	switch(pPacket->sHead.ubType) {

	}
}
