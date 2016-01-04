#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/server.h"

#include "symulacja.h"
#include "config.h"
#include "serverProcess.h"

int main(void) {
	logCreate("symulacja.log");
	memCreate();
	configLoad();
	netCreate();
	simCreate();

	netServerCreate(10, 888, (fnPacketProcess*)serverProcessProtocol, 10);
	netRun();

	simDestroy();
	netDestroy();
	memDestroy();
	logDestroy();
	return 0;
}

UBYTE even(UBYTE x) {
	return x & 1;
}

UBYTE odd(UBYTE x) {
	return x ^ 1;
}

void simCreate(void) {
	UBYTE x,y;
	UBYTE ubPlatform;
	UBYTE (*op)(UBYTE);

  // TODO(#2): Load dimensions from config
  g_sSim.ubWidth = 20;
  g_sSim.ubHeight = 9;

	// Generate storage fields
	logWrite("Generating storage...");
	g_sSim.pFields = memAlloc(g_sSim.ubWidth * sizeof(UBYTE*));
	for(x = g_sSim.ubWidth; x--;) {
		g_sSim.pFields[x] = memAlloc(g_sSim.ubHeight);
		memset(g_sSim.pFields[x], FIELD_EMPTY, g_sSim.ubHeight);
	}

	// Generate platforms
	logWrite("Generating platforms...");
	op = (1?even:odd); // TODO(#2): Read even/odd from config
	g_sSim.ubPlatformCount = (g_sSim.ubHeight >> 1) << 1;
	g_sSim.pPlatforms = memAlloc(g_sSim.ubPlatformCount * sizeof(tPlatform));
	ubPlatform = 0;
	for(y = g_sSim.ubHeight-1; --y;)
		if(op(y)) {
			// Left platform
			platformInit(ubPlatform++, 0, y, PLATFORM_OUT);
			// Right platform
			platformInit(ubPlatform++, g_sSim.ubWidth-1, y, PLATFORM_IN);
		}

	// Generate arms
	logWrite("Generating arms...");
	armInit(ARM_A, 0, 4, 6, g_sSim.ubWidth >> 1);
	armInit(ARM_B, 4, 8, 6, g_sSim.ubHeight >> 1);

	// TODO(#9): Add simulation process timer
	logWrite("Creating update timer...");
	uv_timer_init(g_sNetManager.pLoop, &g_sSim.sTimer);
	uv_timer_start(&g_sSim.sTimer, simUpdate, 500, 500);
	logSuccess("Simulation created");
}

void simDestroy(void) {
	UBYTE x;

	// TODO(#9): Stop simulation process timer

	// Free platforms
  memFree(g_sSim.pPlatforms);

	// Free storage fields
	for(x = g_sSim.ubWidth; x--;)
		memFree(g_sSim.pFields[x]);
	memFree(g_sSim.pFields);

	logSuccess("Simulation destroyed");
}

void simUpdate(uv_timer_t *pTimer) {
	// Update arms
	armUpdate(&g_sSim.sArmA);
	armUpdate(&g_sSim.sArmB);
}

tSim g_sSim;
