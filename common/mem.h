#ifndef COMMON_MEM_H
#define COMMON_MEM_H

#include "types.h"
#include "list.h"

typedef struct _tMemManager{
  ULONG ulMemUsed; /// Current memory usage in bytes
  ULONG ulMaxUsed; /// Peak memory usage in bytes
  tList *pList;    /// Allocation list
  UWORD uwIndent;
  FILE *pLog;
} tMemManager;

typedef struct _tAllocEntry{
	void *pAddr;
	ULONG ulSize;
} tMemAllocEntry;

void memCreate(
	IN char *szLogName
);

void memDestroy(void);

void *memAlloc(ULONG ulSize);

void memFree(void *pPtr);

void memSummary(void);

tMemManager g_sMemManager;

#endif // COMMON_MEM_H
