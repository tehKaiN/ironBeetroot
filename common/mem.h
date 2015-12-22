#ifndef COMMON_MEM_H
#define COMMON_MEM_H

#include "types.h"

typedef struct _tMemManager{
  ULONG ulMemUsed; /// Current memory usage in bytes
  ULONG ulMaxUsed; /// Peak memory usage in bytes
} tMemManager;

void memCreate(void);

void memDestroy(void);

void *memAlloc(ULONG ulSize);

void memFree(void *pPtr);

void memSummary(void);

tMemManager g_sMemManager;

#endif // COMMON_MEM_H
