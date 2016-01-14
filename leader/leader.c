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

	uv_mutex_init(&g_sLeader.sPackageMutex);

	uv_timer_init(g_sNetManager.pLoop, &g_sLeader.sRouteTimer);
	uv_timer_init(g_sNetManager.pLoop, &g_sLeader.sHallTimer);
	uv_timer_start(&g_sLeader.sRouteTimer, armUpdate, 5000, 0);
	uv_timer_start(&g_sLeader.sRouteTimer, packageUpdate, 200, 200);
}

void leaderDestroy(void) {
	uv_timer_stop(&g_sLeader.sRouteTimer);
	uv_timer_stop(&g_sLeader.sHallTimer);

	uv_mutex_destroy(&g_sLeader.sPackageMutex);

	platformFree();
	packageFree();
}

tLeader g_sLeader;
