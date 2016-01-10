#include <uv.h>
#include "mem.h"
#include "list.h"

tList *listCreate(UWORD uwNodeDataSize) {
	tList *pList;

	pList = malloc(sizeof(tList));
	pList->uwNodeDataSize = uwNodeDataSize;
	pList->pHead = 0;
	pList->pTail = 0;
	uv_mutex_init(&pList->sMutex);

	return pList;
}

void listDestroy(tList *pList) {
	// Free all nodes
	listRmAll(pList);

	// Cleanup
	uv_mutex_destroy(&pList->sMutex);
	free(pList);
}


tListNode *listCreateNode(tList *pList) {
	tListNode *pNode;

	pNode = malloc(sizeof(tListNode));
	pNode->pData = malloc(pList->uwNodeDataSize);

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
	if(pList->pTail)
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

  free(pNode->pData);
  free(pNode);
}

void listRmAll(tList *pList) {
	tListNode *pNode, *pNext;

	uv_mutex_lock(&pList->sMutex);
	pNode = pList->pHead;
	while(pNode) {
		pNext = pNode->pNext;
		free(pNode->pData);
		free(pNode);
		pNode = pNext;
	}
	uv_mutex_unlock(&pList->sMutex);
}

tListNode *listGetNodeByData(tList *pList, void *pData) {
	tListNode *pNode;

	uv_mutex_lock(&pList->sMutex);
	pNode = pList->pHead;
	while(pNode) {
    if(pNode->pData == pData) {
			uv_mutex_unlock(&pList->sMutex);
			return pNode;
		}
		pNode = pNode->pNext;
	}
	uv_mutex_unlock(&pList->sMutex);
	return 0;
}
