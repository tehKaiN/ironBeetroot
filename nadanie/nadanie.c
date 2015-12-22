#include "../common/mem.h"
#include "../common/log.h"

#include "nadanie.h"
#include "client.h"
#include "config.h"
#include "process.h"

int main(void) {
	memCreate();
	logSuccess("Init\n");

	configLoad();
	clientRun("127.0.0.1", 888, processClient);

	memDestroy();
	return 0;
}
