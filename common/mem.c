#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "log.h"

// TODO(#7): memory allocation list

void memCreate(void) {
	// Clear mem manager
	memset(&g_sMemManager, 0, sizeof(g_sMemManager));
}

void memDestroy(void) {
	memSummary();
}

void *memAlloc(ULONG ulSize) {
	g_sMemManager.ulMemUsed += ulSize;
	if(g_sMemManager.ulMemUsed > g_sMemManager.ulMaxUsed)
		g_sMemManager.ulMaxUsed = g_sMemManager.ulMemUsed;
	return malloc(ulSize);
}

void memFree(void *pPtr) {
	return free(pPtr);
}

void memSummary(void) {
	logWrite("Mem peak usage: %lu", g_sMemManager.ulMaxUsed);
//	if(g_sMemManager.ulMemUsed)
//		logWarning("%lu bytes remain allocated!", g_sMemManager.ulMemUsed);
}
