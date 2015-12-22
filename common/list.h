#ifndef GUARD_COMMON_LIST_H
#define GUARD_COMMON_LIST_H

#include <uv.h>

typedef struct _tListNode{
	struct _tListNode *pNext;
	void *pData;
} tListNode;

typedef struct _tList{
	tListNode *pHead;     /// First node
	tListNode *pTail;     /// Last node
	UWORD uwNodeDataSize; /// Size of pData in each list node
	uv_mutex_t sMutex;    /// Mutex for list operations
} tList;

tList *listCreate(
	IN UWORD uwNodeDataSize
);

void listDestroy(
	IN tList *pList
);

tListNode *listCreateNode(
	IN tList *pList
);

tListNode *listAddHead(
	IN tList *pList
);

tListNode *listAddTail(
	IN tList *pList
);

void listRmNode(
	IN tList *pList,
	IN tListNode *pNode
);

void listRmAll(
	IN tList *pList
);

#endif // GUARD_COMMON_LIST_H
