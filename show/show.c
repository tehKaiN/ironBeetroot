#include "show.h"
#include "../common/log.h"
#include "../common/mem.h"
#include "../common/net/client.h"
#include "process.h"
#include "gfx.h"

int main(int argc, char* args[]) {
	logCreate("customer.log");
	memCreate("customer_mem.log");
	netCreate();
	gfxCreate();
	showCreate();

	g_sShow.pClientHall = netClientCreate(
		"127.0.0.1", 888,
		showOnConnect, processHallPacket
	);
	g_sShow.pClientLeader = netClientCreate(
		"127.0.0.1", 0x888,
		showOnConnect, processLeaderPacket
	);
	netRun();

	showDestroy();
	gfxDestroy();
	netDestroy();
	memDestroy();
	logDestroy();
	return 0;
}

void showCreate(void) {

	g_sShow.ubReadyHall = 0;
	g_sShow.ubReadyLeader = 0;
	g_sShow.ubCustomerCount = 0;
	g_sShow.ubPlatformCount = 0;
	g_sShow.ubPackageCount = 0;

}

void showDestroy(void) {
}

void showOnConnect(tNetClient *pClient) {
	tPacketSetType sPacket;

	// Send ID packet
	logWrite(
		"Sending client type: '%s\' (%u)",
		g_pClientTypes[CLIENT_TYPE_SHOW], CLIENT_TYPE_SHOW
	);
	packetMakeSetType(&sPacket, CLIENT_TYPE_SHOW);
	netSend(&pClient->sSrvConn, (tPacket*)&sPacket, netReadOnWrite);
}

tShow g_sShow;
