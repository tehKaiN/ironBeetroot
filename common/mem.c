#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "mem.h"
#include "log.h"

/// TODO: memory allocation list

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

void memAllocUvBfr(uv_handle_t *pHandle, size_t ulSuggestedSize, uv_buf_t *pBuf) {
	 *pBuf = uv_buf_init(memAlloc(sizeof(ulSuggestedSize)), ulSuggestedSize);
}
