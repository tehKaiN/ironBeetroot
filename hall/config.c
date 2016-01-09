#include "config.h"

tConfig g_sConfig;

void configLoad(void) {
	// TODO(#2): load from .ini
	g_sConfig.ubBacklogLength = 10;
	g_sConfig.uwServerTimeout = 60;
}
