#include <stdio.h>
#include <stdlib.h>

typedef struct ListNode {
    int val;
    struct ListNode *next;
} ListNode;

/* 构造函数 */
ListNode *newListNode(int val) {
    ListNode *node;
    node = (ListNode *)malloc(sizeof(ListNode));
    // 1. 检查是否申请成功（防止内存耗尽导致解引用空指针崩溃）
    if (node == NULL) {
        // 错误处理，如退出或报错
        return NULL;
    }
    node->val = val;
    node->next = NULL;
    return node;
}

/* 在链表的节点 n0 之后插入节点 P */
void insert(ListNode *n0, ListNode *P) {
    ListNode *n1 = n0->next;
    P->next = n1;
    n0->next = P;
}

/* 删除链表的节点 n0 之后的首个节点 */
// 注意：stdio.h 占用了 remove 关键词
void removeItem(ListNode *n0) {
    if (!n0->next)
        return;
    // n0 -> P -> n1
    ListNode *P = n0->next;
    ListNode *n1 = P->next;
    n0->next = n1;
    // 释放内存
    free(P);
}

/* 访问链表中索引为 index 的节点 */
ListNode *access(ListNode *head, int index) {
    for (int i = 0; i < index; i++) {
        if (head == NULL)
            return NULL;
        head = head->next;
    }
    return head;
}

/* 在链表中查找值为 target 的首个节点 */
int find(ListNode *head, int target) {
    int index = 0;
    while (head) {
        if (head->val == target)
            return index;
        head = head->next;
        index++;
    }
    return -1;
}


/* 打印整个链表 */
void printList(const char *title, ListNode *head) {
    printf("%s: ", title);
    while (head) {
        printf("%d", head->val);
        if (head->next)
            printf(" -> ");
        head = head->next;
    }
    printf(" -> NULL\n");
}

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;

    printf("=== newListNode: 创建节点并连接 ===\n");
    ListNode *n0 = newListNode(1);
    ListNode *n1 = newListNode(3);
    ListNode *n2 = newListNode(2);
    ListNode *n3 = newListNode(5);
    ListNode *n4 = newListNode(4);
    n0->next = n1;
    n1->next = n2;
    n2->next = n3;
    n3->next = n4;
    printList("初始链表", n0);

    printf("\n=== insert: 在 n0 之后插入值为 0 的节点 ===\n");
    ListNode *p = newListNode(6);
    insert(n0, p);
    printList("插入后", n0);

    printf("\n=== access: 访问指定索引的节点 ===\n");
    ListNode *node = access(n0, 3);
    if (node)
        printf("索引 3 的节点值 = %d\n", node->val);
    printf("索引 10 的节点 = %s\n", access(n0, 10) ? "存在" : "NULL(越界)");

    printf("\n=== find: 查找值对应的索引 ===\n");
    printf("查找值 5 -> 索引 %d\n", find(n0, 5));
    printf("查找值 99 -> 索引 %d (-1 表示未找到)\n", find(n0, 99));

    printf("\n=== removeItem: 删除 n0 之后的首个节点 ===\n");
    removeItem(n0);
    printList("删除后", n0);

    // 释放全部节点
    while (n0) {
        ListNode *next = n0->next;
        free(n0);
        n0 = next;
    }
    return 0;
}
