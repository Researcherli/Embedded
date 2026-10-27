#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

/* 基于环形数组实现的队列 */
typedef struct {
    int *nums;       // 用于存储队列元素的数组
    int front;       // 队首指针，指向队首元素
    int queSize;     // 当前队列的元素数量
    int queCapacity; // 队列容量
} ArrayQueue;

/* 构造函数 */
ArrayQueue *newArrayQueue(int capacity) {
    ArrayQueue *queue = (ArrayQueue *)malloc(sizeof(ArrayQueue));
    // 初始化数组
    queue->queCapacity = capacity;
    queue->nums = (int *)malloc(sizeof(int) * queue->queCapacity);
    queue->front = queue->queSize = 0;
    return queue;
}

/* 析构函数 */
void delArrayQueue(ArrayQueue *queue) {
    free(queue->nums);
    free(queue);
}

/* 获取队列的容量 */
int capacity(ArrayQueue *queue) {
    return queue->queCapacity;
}

/* 获取队列的长度 */
int size(ArrayQueue *queue) {
    return queue->queSize;
}

/* 判断队列是否为空 */
bool empty(ArrayQueue *queue) {
    return queue->queSize == 0;
}

/* 访问队首元素 */
int peek(ArrayQueue *queue) {
    assert(size(queue) != 0);
    return queue->nums[queue->front];
}

/* 入队 */
void push(ArrayQueue *queue, int num) {
    if (size(queue) == capacity(queue)) {
        printf("队列已满\r\n");
        return;
    }
    // 计算队尾指针，指向队尾索引 + 1
    // 通过取余操作实现 rear 越过数组尾部后回到头部
    int rear = (queue->front + queue->queSize) % queue->queCapacity;
    // 将 num 添加至队尾
    queue->nums[rear] = num;
    queue->queSize++;
}

/* 出队 */
int pop(ArrayQueue *queue) {
    int num = peek(queue);
    // 队首指针向后移动一位，若越过尾部，则返回到数组头部
    queue->front = (queue->front + 1) % queue->queCapacity;
    queue->queSize--;
    return num;
}

/* 返回数组用于打印 */
int *toArray(ArrayQueue *queue, int *queSize) {
    *queSize = queue->queSize;
    int *res = (int *)calloc(queue->queSize, sizeof(int));
    int j = queue->front;
    for (int i = 0; i < queue->queSize; i++) {
        res[i] = queue->nums[j % queue->queCapacity];
        j++;
    }
    return res;
}

int main(void) {
    printf("1. 创建容量为 5 的数组队列\n");
    ArrayQueue *q = newArrayQueue(5);

    printf("2. 依次入队 1, 2, 3\n");
    push(q, 1);
    push(q, 2);
    push(q, 3);

    printf("3. 大小=%d, 容量=%d, 是否为空=%d\n", size(q), capacity(q), empty(q));
    printf("4. 查看队首元素: %d\n", peek(q));

    printf("5. 出队: %d\n", pop(q));
    printf("6. 出队后队首: %d, 大小=%d\n", peek(q), size(q));

    printf("7. 再入队 4, 5, 6（队尾绕回数组开头）\n");
    push(q, 4);
    push(q, 5);
    push(q, 6);

    printf("8. 再入队 7（队列已满，应提示失败）\n");
    push(q, 7);

    printf("9. 按顺序取出所有元素: ");
    int n;
    int *arr = toArray(q, &n);
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    free(arr);

    printf("10. 释放队列\n");
    delArrayQueue(q);
    return 0;
}
