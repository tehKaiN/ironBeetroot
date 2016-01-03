#ifndef COMMON_MEM_H
#define COMMON_MEM_H

#include "types.h"
#include <uv.h>

typedef struct _tMemManager{
  ULONG ulMemUsed; /// Current memory usage in bytes
  ULONG ulMaxUsed; /// Peak memory usage in bytes
} tMemManager;

void memCreate(void);

void memDestroy(void);

void *memAlloc(ULONG ulSize);

void memFree(void *pPtr);

void memSummary(void);

// TODO: move this fn elsewhere
void memAllocUvBfr(
	IN uv_handle_t *pHandle,
	IN size_t ulSuggestedSize,
	OUT uv_buf_t *pBuf
);

tMemManager g_sMemManager;

#endif // COMMON_MEM_H
