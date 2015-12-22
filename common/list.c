#include <uv.h>
#include "mem.h"
#include "list.h"

tList *listCreate(UWORD uwNodeDataSize) {
	tList *pList;

	pList = memAlloc(sizeof(tList));
	pList->uwNodeDataSize = uwNodeDataSize;
	pList->pHead = 0;
	pList->pTail = 0;
	uv_mutex_init(&pList->sMutex);

	return pList;
}

void listDestroy(tList *pList) {
	// TODO: free all nodes
	listRmAll(pList);

	// cleanup
	uv_mutex_destroy(&pList->sMutex);
	memFree(pList);
}


tListNode *listCreateNode(tList *pList) {
	tListNode *pNode;

	pNode = memAlloc(sizeof(tListNode));
	pNode->pData = memAlloc(pList->uwNodeDataSize);

	return pNode;
}

/**
 * Adds node as head of list - previous head is next
 */
tListNode *listAddHead(tList *pList) {
	tListNode *pNode;

	pNode = listCreateNode(pList);
	uv_mutex_lock(&pList->sMutex);
	pNode->pNext = pList->pHead;
	pList->pHead = pNode;
	if(!pList->pTail)
		pList->pTail = pNode;
	uv_mutex_unlock(&pList->sMutex);

	return pNode;
}

/**
 * Adds node as tail of list - previous tail is prev
 */
tListNode *listAddTail(tList *pList) {
	tListNode *pNode;

	pNode = listCreateNode(pList);
	uv_mutex_lock(&pList->sMutex);
	pList->pTail->pNext = pNode;
	pList->pTail = pNode;
	pNode->pNext = 0;
	if(!pList->pHead)
		pList->pHead = pNode;
	uv_mutex_unlock(&pList->sMutex);

	return pNode;
}

void listRmNode(tList *pList, tListNode *pNode) {
	tListNode *pPrev;

	uv_mutex_lock(&pList->sMutex);
  if(pList->pHead == pNode)
		pPrev = 0;
	else {
		pPrev = pList->pHead;
		while(pPrev->pNext != pNode)
			pPrev = pPrev->pNext;
	}

  if(pList->pTail == pNode) {
		pList->pTail = pPrev;
  }
  if(pList->pHead == pNode) {
		pList->pHead = pNode->pNext;
  }
  if(pPrev)
		pPrev->pNext = pNode->pNext;
	uv_mutex_unlock(&pList->sMutex);

  memFree(pNode->pData);
  memFree(pNode);
}

void listRmAll(tList *pList) {
	tListNode *pNode, *pNext;

	uv_mutex_lock(&pList->sMutex);
	pNode = pList->pHead;
	while(pNode) {
		pNext = pNode->pNext;
		memFree(pNode->pData);
		memFree(pNode);
		pNode = pNext;
	}
	uv_mutex_unlock(&pList->sMutex);
}
