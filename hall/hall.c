#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/server.h"

#include "hall.h"
#include "config.h"
#include "serverProcess.h"

int main(void) {
	logCreate("hall.log");
	memCreate();
	configLoad();
	netCreate();
	hallCreate();

	g_sHall.pServer = netServerCreate(
		10, 888, (fnPacketProcess*)serverProcessProtocol, 10
	);
	netRun();

	hallDestroy();
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

void hallCreate(void) {
	UBYTE x,y;
	UBYTE ubPlatform;
	UBYTE (*op)(UBYTE);

  // TODO(#2): Load dimensions from config
  g_sHall.ubWidth = 20;
  g_sHall.ubHeight = 9;

	// Generate storage fields
	logWrite("Generating storage...");
	g_sHall.pFields = memAlloc(g_sHall.ubWidth * sizeof(UBYTE*));
	for(x = g_sHall.ubWidth; x--;) {
		g_sHall.pFields[x] = memAlloc(g_sHall.ubHeight);
		memset(g_sHall.pFields[x], FIELD_EMPTY, g_sHall.ubHeight);
	}

	// Generate platforms
	logWrite("Generating platforms...");
	op = (1?even:odd); // TODO(#2): Read even/odd from config
	g_sHall.ubPlatformCount = (g_sHall.ubHeight >> 1) << 1;
	g_sHall.pPlatforms = memAlloc(g_sHall.ubPlatformCount * sizeof(tPlatform));
	ubPlatform = 0;
	for(y = g_sHall.ubHeight-1; --y;)
		if(op(y)) {
			// Left platform
			platformInit(ubPlatform++, 0, y, PLATFORM_OUT);
			// Right platform
			platformInit(ubPlatform++, g_sHall.ubWidth-1, y, PLATFORM_IN);
		}

	// Generate arms
	logWrite("Generating arms...");
	armInit(ARM_A, 0, 4, 6, g_sHall.ubWidth >> 1);
	armInit(ARM_B, 4, 8, 6, g_sHall.ubHeight >> 1);

	logSuccess("Simulation created");
}

void hallDestroy(void) {
	UBYTE x;

	// TODO(#9): Stop simulation process timer

	// Free platforms
  memFree(g_sHall.pPlatforms);

	// Free storage fields
	for(x = g_sHall.ubWidth; x--;)
		memFree(g_sHall.pFields[x]);
	memFree(g_sHall.pFields);

	logSuccess("Simulation destroyed");
}

tHall g_sHall;
