#include "leader.h"
#include "../common/log.h"
#include "../common/mem.h"
#include "../common/net/net.h"
#include "../common/net/server.h"
#include "../common/net/client.h"

#include "process.h"
#include "config.h"
#include "arm.h"

int main(void) {
	logCreate("leader.log");
	memCreate("leader_mem.log");
	configLoad();
	netCreate();
	leaderCreate();

	g_sLeader.pServer = netServerCreate(
		10, 0x888, processServerPacket, 10
	);

	g_sLeader.pClient = netClientCreate(
		"127.0.0.1", 888, processClientOnConnect, processClientPacket
	);

	netRun();

	leaderDestroy();
	netDestroy();
	memDestroy();
	logDestroy();
	return 0;
}

void leaderCreate(void) {
	g_sLeader.ubReady = 0;
	g_sLeader.ubPlatformCount = 0;

	armInit(&g_sLeader.sArmA, ARM_ID_A);
	armInit(&g_sLeader.sArmB, ARM_ID_B);

	uv_mutex_init(&g_sLeader.sPackageMutex);
	uv_mutex_init(&g_sLeader.sRouteMutex);

	uv_timer_init(g_sNetManager.pLoop, &g_sLeader.sRouteTimer);
	uv_timer_init(g_sNetManager.pLoop, &g_sLeader.sHallTimer);
	uv_timer_init(g_sNetManager.pLoop, &g_sLeader.sArmPosTimer);
	uv_timer_start(&g_sLeader.sRouteTimer, armUpdate, 1000, 1000);
	uv_timer_start(&g_sLeader.sHallTimer, packageUpdate, 200, 200);
	uv_timer_start(&g_sLeader.sArmPosTimer, armPosUpdate, 200, 200);
}

void leaderDestroy(void) {
	uv_timer_stop(&g_sLeader.sRouteTimer);
	uv_timer_stop(&g_sLeader.sHallTimer);
	uv_timer_stop(&g_sLeader.sArmA.sTimer);
	uv_timer_stop(&g_sLeader.sArmB.sTimer);
	uv_timer_stop(&g_sLeader.sArmPosTimer);

	uv_mutex_destroy(&g_sLeader.sArmA.sMutex);
	uv_mutex_destroy(&g_sLeader.sArmB.sMutex);
	uv_mutex_destroy(&g_sLeader.sPackageMutex);
	uv_mutex_destroy(&g_sLeader.sRouteMutex);

	platformFree();
	packageFree();
}

tLeader g_sLeader;
