#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/server.h"

#include "symulacja.h"
#include "config.h"
#include "serverProcess.h"

int main(void) {
	memCreate();
	logSuccess("Init");
	configLoad();
	netCreate();

	netServerCreate(10, 888, (fnPacketProcess*)serverProcessProtocol, 10);
	netRun();

	netDestroy();
	memDestroy();
	return 0;
}
