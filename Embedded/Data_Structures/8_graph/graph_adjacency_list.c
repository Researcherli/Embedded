#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10

/* 顶点结构体 */
typedef struct Vertex {
    int val;
} Vertex;

/* 构造函数 */
Vertex *newVertex(int val) {
    Vertex *vet = (Vertex *)malloc(sizeof(Vertex));
    vet->val = val;
    return vet;
}

/* 节点结构体 */
typedef struct AdjListNode {
    Vertex *vertex;           // 顶点
    struct AdjListNode *next; // 后继节点
} AdjListNode;

/* 添加边辅助函数 */
void addEdgeHelper(AdjListNode *head, Vertex *vet) {
    AdjListNode *node = (AdjListNode *)malloc(sizeof(AdjListNode));
    node->vertex = vet;
    // 头插法
    node->next = head->next;
    head->next = node;
}

/* 删除边辅助函数 */
void removeEdgeHelper(AdjListNode *head, Vertex *vet) {
    AdjListNode *pre = head;
    AdjListNode *cur = head->next;
    // 在链表中搜索 vet 对应节点
    while (cur != NULL && cur->vertex != vet) {
        pre = cur;
        cur = cur->next;
    }
    if (cur == NULL)
        return;
    // 将 vet 对应节点从链表中删除
    pre->next = cur->next;
    // 释放内存
    free(cur);
}

/* 基于邻接表实现的无向图类 */
typedef struct {
    AdjListNode *heads[MAX_SIZE]; // 节点数组
    int size;                     // 节点数量
} GraphAdjList;

/* 构造函数 */
GraphAdjList *newGraphAdjList() {
    GraphAdjList *graph = (GraphAdjList *)malloc(sizeof(GraphAdjList));
    if (!graph) {
        return NULL;
    }
    graph->size = 0;
    for (int i = 0; i < MAX_SIZE; i++) {
        graph->heads[i] = NULL;
    }
    return graph;
}

/* 析构函数 */
void delGraphAdjList(GraphAdjList *graph) {
    for (int i = 0; i < graph->size; i++) {
        AdjListNode *cur = graph->heads[i];
        while (cur != NULL) {
            AdjListNode *next = cur->next;
            if (cur != graph->heads[i]) {
                free(cur);
            }
            cur = next;
        }
        free(graph->heads[i]->vertex);
        free(graph->heads[i]);
    }
    free(graph);
}

/* 查找顶点对应的节点 */
AdjListNode *findNode(GraphAdjList *graph, Vertex *vet) {
    for (int i = 0; i < graph->size; i++) {
        if (graph->heads[i]->vertex == vet) {
            return graph->heads[i];
        }
    }
    return NULL;
}

/* 添加边 */
void addEdge(GraphAdjList *graph, Vertex *vet1, Vertex *vet2) {
    AdjListNode *head1 = findNode(graph, vet1);
    AdjListNode *head2 = findNode(graph, vet2);
    assert(head1 != NULL && head2 != NULL && head1 != head2);
    // 添加边 vet1 - vet2
    addEdgeHelper(head1, vet2);
    addEdgeHelper(head2, vet1);
}

/* 删除边 */
void removeEdge(GraphAdjList *graph, Vertex *vet1, Vertex *vet2) {
    AdjListNode *head1 = findNode(graph, vet1);
    AdjListNode *head2 = findNode(graph, vet2);
    assert(head1 != NULL && head2 != NULL);
    // 删除边 vet1 - vet2
    removeEdgeHelper(head1, head2->vertex);
    removeEdgeHelper(head2, head1->vertex);
}

/* 添加顶点 */
void addVertex(GraphAdjList *graph, Vertex *vet) {
    assert(graph != NULL && graph->size < MAX_SIZE);
    AdjListNode *head = (AdjListNode *)malloc(sizeof(AdjListNode));
    head->vertex = vet;
    head->next = NULL;
    // 在邻接表中添加一个新链表
    graph->heads[graph->size++] = head;
}

/* 删除顶点 */
void removeVertex(GraphAdjList *graph, Vertex *vet) {
    AdjListNode *node = findNode(graph, vet);
    assert(node != NULL);
    AdjListNode *cur, *pre;
    // 遍历其他顶点的链表，删除所有包含 vet 的边
    for (int i = 0; i < graph->size; i++) {
        cur = graph->heads[i];
        pre = NULL;
        while (cur) {
            pre = cur;
            cur = cur->next;
            if (cur && cur->vertex == vet) {
                pre->next = cur->next;
                free(cur);
                break;
            }
        }
    }
    // 将该顶点之后的顶点向前移动，以填补空缺
    int i;
    for (i = 0; i < graph->size; i++) {
        if (graph->heads[i] == node)
            break;
    }
    for (int j = i; j < graph->size - 1; j++) {
        graph->heads[j] = graph->heads[j + 1];
    }
    graph->size--;
    // 在邻接表中删除顶点 vet 对应的链表（须在上面的遍历之后释放）
    cur = node;
    while (cur) {
        pre = cur;
        cur = cur->next;
        free(pre);
    }
    free(vet);
}

#ifndef GRAPH_ADJ_LIST_NO_MAIN
int main(void) {
    printf("1. 创建邻接表图，添加顶点 1, 2, 3\n");
    GraphAdjList *graph = newGraphAdjList();
    Vertex *v[3];
    for (int i = 0; i < 3; i++) {
        v[i] = newVertex(i + 1);
        addVertex(graph, v[i]);
    }

    printf("2. 添加边 1-2, 2-3\n");
    addEdge(graph, v[0], v[1]);
    addEdge(graph, v[1], v[2]);
    printf("   顶点 2 的邻接顶点: ");
    for (AdjListNode *node = findNode(graph, v[1])->next; node != NULL; node = node->next) {
        printf("%d ", node->vertex->val);
    }
    printf("\n");

    printf("3. 删除边 1-2\n");
    removeEdge(graph, v[0], v[1]);
    printf("   顶点 2 的邻接顶点: ");
    for (AdjListNode *node = findNode(graph, v[1])->next; node != NULL; node = node->next) {
        printf("%d ", node->vertex->val);
    }
    printf("\n");

    printf("4. 删除顶点 1，剩余顶点数=");
    removeVertex(graph, v[0]);
    printf("%d\n", graph->size);

    printf("5. 释放图\n");
    delGraphAdjList(graph);
    return 0;
}
#endif
