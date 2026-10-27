#ifndef LIST_H
#define LIST_H

#include <stddef.h> // 补上 NULL 的定义
#include <stdint.h>


/* 绑定与获取节点的拥有者（即 TCB 任务控制块指针） */
#define listSET_LIST_ITEM_OWNER(pxListItem, pxOwner)                           \
    ((pxListItem)->pvOwner = (void *)(pxOwner))
#define listGET_LIST_ITEM_OWNER(pxListItem) ((pxListItem)->pvOwner)

/* 设置与获取节点的排序值（如唤醒时间点） */
#define listSET_LIST_ITEM_VALUE(pxListItem, xValue)                            \
    ((pxListItem)->xItemValue = (xValue))
#define listGET_LIST_ITEM_VALUE(pxListItem) ((pxListItem)->xItemValue)

/* 获取链表头节点、尾哨兵节点、下一个节点 */
#define listGET_HEAD_ENTRY(pxList) (((pxList)->xListEnd).pxNext)
#define listGET_END_MARKER(pxList) ((ListItem_t const *)(&((pxList)->xListEnd)))
#define listGET_NEXT(pxListItem) ((pxListItem)->pxNext)

struct xLIST;
struct xLIST_ITEM {
    uint16_t xItemValue;
    struct xLIST_ITEM *pxNext;
    struct xLIST_ITEM *pxPrevious;
    void *pvOwner;
    struct xLIST *pxContainer;
};
typedef struct xLIST_ITEM ListItem_t;

typedef struct xLIST_ITEM MiniListItem_t;


typedef struct xLIST {
    unsigned long uxNumberOfItems;
    ListItem_t *pxIndex;
    MiniListItem_t xListEnd;
} List_t;

void vListInitialise(List_t *const pxList) ;
void vListInitialiseItem(ListItem_t *const pxItem) ;
void vListInsert(List_t *const pxList, ListItem_t *const pxNewListItem) ;
void vListInsertEnd(List_t *const pxList, ListItem_t *const pxNewListItem) ;
unsigned long uxListRemove(ListItem_t *const pxItemToRemove);

#endif /* ifndef LIST_H */
