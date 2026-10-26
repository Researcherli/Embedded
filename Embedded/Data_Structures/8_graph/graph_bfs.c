#define GRAPH_ADJ_LIST_NO_MAIN
#include "graph_adjacency_list.c"

/* 节点队列结构体 */
typedef struct {
    Vertex *vertices[MAX_SIZE];
    int front, rear, size;
} Queue;

/* 构造函数 */
Queue *newQueue() {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->front = q->rear = q->size = 0;
    return q;
}

/* 判断队列是否为空 */
int isEmpty(Queue *q) { return q->size == 0; }

/* 入队操作 */
void enqueue(Queue *q, Vertex *vet) {
    q->vertices[q->rear] = vet;
    q->rear = (q->rear + 1) % MAX_SIZE;
    q->size++;
}

/* 出队操作 */
Vertex *dequeue(Queue *q) {
    Vertex *vet = q->vertices[q->front];
    q->front = (q->front + 1) % MAX_SIZE;
    q->size--;
    return vet;
}

/* 检查顶点是否已被访问 */
int isVisited(Vertex **visited, int size, Vertex *vet) {
    // 遍历查找节点，使用 O(n) 时间
    for (int i = 0; i < size; i++) {
        if (visited[i] == vet)
            return 1;
    }
    return 0;
}

/* 广度优先遍历 */
// 使用邻接表来表示图，以便获取指定顶点的所有邻接顶点
void graphBFS(GraphAdjList *graph, Vertex *startVet, Vertex **res, int *resSize,
              Vertex **visited, int *visitedSize) {
    // 队列用于实现 BFS
    Queue *queue = newQueue();
    enqueue(queue, startVet);
    visited[(*visitedSize)++] = startVet;
    // 以顶点 vet 为起点，循环直至访问完所有顶点
    while (!isEmpty(queue)) {
        Vertex *vet = dequeue(queue); // 队首顶点出队
        res[(*resSize)++] = vet;      // 记录访问顶点
        // 遍历该顶点的所有邻接顶点
        AdjListNode *node = findNode(graph, vet);
        while (node != NULL) {
            // 跳过已被访问的顶点
            if (!isVisited(visited, *visitedSize, node->vertex)) {
                enqueue(queue, node->vertex);             // 只入队未访问的顶点
                visited[(*visitedSize)++] = node->vertex; // 标记该顶点已被访问
            }
            node = node->next;
        }
    }
    // 释放内存
    free(queue);
}

/* 深度优先遍历辅助函数 */
void dfs(GraphAdjList *graph, Vertex **res, int *resSize, Vertex *vet) {
    // 记录访问顶点
    res[(*resSize)++] = vet;
    // 遍历该顶点的所有邻接顶点
    AdjListNode *node = findNode(graph, vet);
    while (node != NULL) {
        // 跳过已被访问的顶点
        if (!isVisited(res, *resSize, node->vertex)) {
            // 递归访问邻接顶点
            dfs(graph, res, resSize, node->vertex);
        }
        node = node->next;
    }
}

/* 深度优先遍历 */
// 使用邻接表来表示图，以便获取指定顶点的所有邻接顶点
void graphDFS(GraphAdjList *graph, Vertex *startVet, Vertex **res,
              int *resSize) {
    dfs(graph, res, resSize, startVet);
}

/* 打印顶点序列 */
void printVertices(Vertex **vets, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", vets[i]->val);
    }
    printf("\n");
}

int main(void) {
    printf("1. 创建无向图，添加顶点 1..5\n");
    GraphAdjList *graph = newGraphAdjList();
    Vertex *v[5];
    for (int i = 0; i < 5; i++) {
        v[i] = newVertex(i + 1);
        addVertex(graph, v[i]);
    }

    printf("2. 添加边 1-2, 1-3, 2-4, 3-4, 4-5\n");
    addEdge(graph, v[0], v[1]);
    addEdge(graph, v[0], v[2]);
    addEdge(graph, v[1], v[3]);
    addEdge(graph, v[2], v[3]);
    addEdge(graph, v[3], v[4]);

    Vertex *res[MAX_SIZE];
    Vertex *visited[MAX_SIZE];
    int resSize = 0, visitedSize = 0;

    printf("3. 从顶点 1 开始广度优先遍历(BFS): ");
    graphBFS(graph, v[0], res, &resSize, visited, &visitedSize);
    printVertices(res, resSize);

    resSize = 0;
    printf("4. 从顶点 1 开始深度优先遍历(DFS): ");
    graphDFS(graph, v[0], res, &resSize);
    printVertices(res, resSize);

    printf("5. 删除边 2-4\n");
    removeEdge(graph, v[1], v[3]);
    resSize = 0;
    printf("   删除后 DFS: ");
    graphDFS(graph, v[0], res, &resSize);
    printVertices(res, resSize);

    printf("6. 删除顶点 3\n");
    removeVertex(graph, v[2]);
    printf("   顶点数量=%d\n", graph->size);

    printf("7. 释放图\n");
    delGraphAdjList(graph);
    return 0;
}



