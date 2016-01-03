#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/net/client.h"
#include "nadanie.h"
#include "config.h"
#include "process.h"

int main(void) {
	memCreate();
	logSuccess("Init");
	netCreate();

	netClientCreate("127.0.0.1", 888, processClient);
	netRun();

	netDestroy();
	memDestroy();
	return 0;
}
