#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/net/client.h"
#include "../common/packet.h"
// TODO(#2): config
#include "arm.h"
#include "process.h"

int main(void){
	logCreate("arm.log");
	memCreate("arm_mem.loog");
	netCreate();
	armCreate();

	g_sArm.pClientHall = netClientCreate("127.0.0.1", 888, armOnConnect, processHallPacket);
	g_sArm.pClientLeader = netClientCreate("127.0.0.1", 0x888, armOnConnect, processLeaderPacket);
	netRun();

	armDestroy();
	netDestroy();
	memDestroy();
	logDestroy();
	return 0;
}

void armCreate(void) {
	g_sArm.ubReady = 0;

	uv_mutex_init(&g_sArm.sSensorMutex);
	uv_mutex_init(&g_sArm.sCmdMutex);

	uv_timer_init(g_sNetManager.pLoop, &g_sArm.sSensorTimer);
	uv_timer_start(&g_sArm.sSensorTimer, armSensorUpdate, 200, 200);
}

void armDestroy(void) {
	uv_mutex_destroy(&g_sArm.sSensorMutex);
	uv_mutex_destroy(&g_sArm.sCmdMutex);

	uv_timer_stop(&g_sArm.sSensorTimer);
}

void armOnConnect(tNetClient *pClient) {
	tPacketSetType sPacket;

	// Send ID packet
	logWrite(
		"Sending client type: '%s\' (%u)",
		g_pClientTypes[CLIENT_TYPE_ARM], CLIENT_TYPE_ARM
	);

	g_sArm.ubReady = 0;
	packetMakeSetType(&sPacket, CLIENT_TYPE_ARM);
	netSend(&pClient->sSrvConn, (tPacket*)&sPacket, netNopOnWrite);
}

void armSensorUpdate(uv_timer_t *pTimer) {
  tPacketHead sPacket;

	if(!(g_sArm.ubReady & READY_ID_HALL))
		return;

  packetPrepare(
		(tPacket*)&sPacket, PACKET_GETSENSORINFO, sizeof(tPacketHead)
	);

	netSend(&g_sArm.pClientHall->sSrvConn, (tPacket*)&sPacket, netNopOnWrite);
}

tArm g_sArm;
