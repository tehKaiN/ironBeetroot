#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "log.h"

void memCreate(char *szLogName) {
	memset(&g_sMemManager, 0, sizeof(g_sMemManager));
	g_sMemManager.pList = listCreate(sizeof(tMemAllocEntry));
	g_sMemManager.pLog = fopen(szLogName, "w");
	logSuccess("memory log %s started", szLogName);
}

void memDestroy(void) {
	memSummary();
	fclose(g_sMemManager.pLog);
}

void *memAlloc(ULONG ulSize) {
	void *pAddr;
	tListNode *pNode;
	tMemAllocEntry *pEntry;
	UWORD i;

	g_sMemManager.ulMemUsed += ulSize;
	if(g_sMemManager.ulMemUsed > g_sMemManager.ulMaxUsed)
		g_sMemManager.ulMaxUsed = g_sMemManager.ulMemUsed;
	pAddr = malloc(ulSize);
	pNode = listAddTail(g_sMemManager.pList);
	pEntry = (tMemAllocEntry*)pNode->pData;
	pEntry->pAddr = pAddr;
	pEntry->ulSize = ulSize;
	for(i = 0; i != g_sMemManager.uwIndent; ++i)
		fprintf(g_sMemManager.pLog, "\t");
	fprintf(g_sMemManager.pLog, "+ %p (%u)\n", pAddr, ulSize);
	fflush(g_sMemManager.pLog);
	++g_sMemManager.uwIndent;

	return pAddr;
}

void memFree(void *pAddr) {
	tListNode *pNode;
	tMemAllocEntry *pEntry;
	UBYTE i;

	pNode = g_sMemManager.pList->pHead;
	while(pNode) {
		pEntry = pNode->pData;
		if(pEntry->pAddr == pAddr) {
			--g_sMemManager.uwIndent;
			for(i = 0; i != g_sMemManager.uwIndent; ++i)
				fprintf(g_sMemManager.pLog, "\t");
			fprintf(g_sMemManager.pLog, "- %p (%u)\n", pAddr, pEntry->ulSize);
			fflush(g_sMemManager.pLog);
      listRmNode(g_sMemManager.pList, pNode);
			free(pAddr);
			return;
		}
		pNode = pNode->pNext;
	}
}

void memSummary(void) {
	logWrite("Mem peak usage: %lu", g_sMemManager.ulMaxUsed);
//	if(g_sMemManager.ulMemUsed)
//		logWarning("%lu bytes remain allocated!", g_sMemManager.ulMemUsed);
}
