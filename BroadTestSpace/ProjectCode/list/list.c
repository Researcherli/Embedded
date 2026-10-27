#include <stdint.h>
#include "list.h"


/*-----------------------------------------------------------*/
void vListInitialise(List_t *const pxList) {
    pxList->pxIndex = (ListItem_t *)&(pxList->xListEnd);
    pxList->xListEnd.xItemValue = 0xffff;
    pxList->xListEnd.pxNext = (ListItem_t *)&(pxList->xListEnd);
    pxList->xListEnd.pxPrevious = (ListItem_t *)&(pxList->xListEnd);
    pxList->uxNumberOfItems = (unsigned long)0U;
}
/*-----------------------------------------------------------*/

void vListInitialiseItem(ListItem_t *const pxItem) {
    pxItem->pxContainer = NULL;
}
/*-----------------------------------------------------------*/

void vListInsertEnd(List_t *const pxList, ListItem_t *const pxNewListItem) {
    ListItem_t *const pxIndex = pxList->pxIndex;
    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;
    pxIndex->pxPrevious->pxNext = pxNewListItem;
    pxIndex->pxPrevious = pxNewListItem;
    pxNewListItem->pxContainer = pxList;
    (pxList->uxNumberOfItems) = (unsigned long)(pxList->uxNumberOfItems + 1U);
}
/*-----------------------------------------------------------*/

void vListInsert(List_t *const pxList, ListItem_t *const pxNewListItem) {
    ListItem_t *pxIterator;
    const uint16_t xValueOfInsertion = pxNewListItem->xItemValue;

    if (xValueOfInsertion == 0xffff) {
        pxIterator = pxList->xListEnd.pxPrevious;
    } else {
        for (pxIterator = (ListItem_t *)&(pxList->xListEnd);
             pxIterator->pxNext->xItemValue <= xValueOfInsertion;
             pxIterator = pxIterator->pxNext) {
        }
    }

    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = pxNewListItem;
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = pxNewListItem;

    pxNewListItem->pxContainer = pxList;
    (pxList->uxNumberOfItems) = (unsigned long)(pxList->uxNumberOfItems + 1U);
}
/*-----------------------------------------------------------*/

unsigned long uxListRemove(ListItem_t *const pxItemToRemove) {
    List_t *const pxList = pxItemToRemove->pxContainer;

    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;

    if (pxList->pxIndex == pxItemToRemove) {
        pxList->pxIndex = pxItemToRemove->pxPrevious;
    }

    pxItemToRemove->pxContainer = NULL;
    (pxList->uxNumberOfItems) = (uint16_t)(pxList->uxNumberOfItems - 1U);
    return pxList->uxNumberOfItems;
}
/*-----------------------------------------------------------*/
