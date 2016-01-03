#include "../common/mem.h"
#include "../common/log.h"

#include "symulacja.h"
#include "config.h"
#include "serverProcess.h"

int main(void) {
	memCreate();
	logSuccess("Init\n");
	configLoad();
	netCreate();

	netServerCreate(10, 888, serverProcessProtocol, 10);
	netRun();

	netDestroy();
	memDestroy();
	return 0;
}
