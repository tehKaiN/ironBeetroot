#include "arm.h"
#include <string.h>
#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/net/client.h"
#include "../common/packet.h"
// TODO(#2): config
#include "process.h"

int main(LONG lArgCount, char *szArgs[]){
	logCreate("arm.log");
	memCreate("arm_mem.log");
	netCreate();
	armCreate();

	if(lArgCount < 2) {
		logError("No arm type specified (add A/B arg to executable)");
		return 1;
	}

	if(!strcmp(szArgs[1], "A"))
		g_sArm.ubId = ARM_ID_A;
	else if(!strcmp(szArgs[1], "B"))
		g_sArm.ubId = ARM_ID_B;
	else {
		g_sArm.ubId = ARM_ID_ILLEGAL;
		logWrite("Unknown arm type: %s", szArgs[1]);
		return 1;
	}

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
	g_sArm.ubState = ARM_STATE_OPEN | ARM_STATE_UP;
	g_sArm.ubCmdState = ARM_CMDSTATE_IDLE;

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
	sPacket.ubExtra = g_sArm.ubId;
	netSend(&pClient->sSrvConn, (tPacket*)&sPacket, netReadOnWrite);
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
