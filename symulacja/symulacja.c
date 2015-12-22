#include "../common/mem.h"
#include "../common/log.h"

#include "symulacja.h"
#include "server.h"
#include "config.h"
#include "serverProcess.h"

int main(void) {
	memCreate();
	logSuccess("Init\n");

	configLoad();
	serverRun(10, 888, serverProcessProtocol);

	memDestroy();
	return 0;
}
