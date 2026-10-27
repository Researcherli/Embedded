#define HEAP_NO_MAIN
#include "my_heap.c"

/* 元素入堆 */
void pushMinHeap(MaxHeap *maxHeap, int val) {
    // 元素取反
    push(maxHeap, -val);
}

/* 元素出堆 */
int popMinHeap(MaxHeap *maxHeap) {
    // 元素取反
    return -pop(maxHeap);
}

/* 访问堆顶元素 */
int peekMinHeap(MaxHeap *maxHeap) {
    // 元素取反
    return -peek(maxHeap);
}

/* 取出堆中元素 */
int *getMinHeap(MaxHeap *maxHeap) {
    // 将堆中所有元素取反并存入 res 数组
    int *res = (int *)malloc(maxHeap->size * sizeof(int));
    for (int i = 0; i < maxHeap->size; i++) {
        res[i] = -maxHeap->data[i];
    }
    return res;
}

// 基于堆查找数组中最大的 k 个元素的函数
int *topKHeap(int *nums, int sizeNums, int k) {
    // 初始化小顶堆
    // 请注意：我们将堆中所有元素取反，从而用大顶堆来模拟小顶堆
    int *empty = (int *)malloc(0);
    MaxHeap *maxHeap = newMaxHeap(empty, 0);
    // 将数组的前 k 个元素入堆
    for (int i = 0; i < k; i++) {
        pushMinHeap(maxHeap, nums[i]);
    }
    // 从第 k+1 个元素开始，保持堆的长度为 k
    for (int i = k; i < sizeNums; i++) {
        // 若当前元素大于堆顶元素，则将堆顶元素出堆、当前元素入堆
        if (nums[i] > peekMinHeap(maxHeap)) {
            popMinHeap(maxHeap);
            pushMinHeap(maxHeap, nums[i]);
        }
    }
    int *res = getMinHeap(maxHeap);
    // 释放内存
    delMaxHeap(maxHeap);
    return res;
}

/* 打印序列 */
void printSeq(int *seq, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", seq[i]);
    }
    printf("\n");
}

int main(void) {
    printf("1. 建立小顶堆（内部用取反的大顶堆实现）\n");
    int empty[1];
    MaxHeap *heap = newMaxHeap(empty, 0);

    printf("2. 依次入堆 5, 1, 8, 3\n");
    pushMinHeap(heap, 5);
    pushMinHeap(heap, 1);
    pushMinHeap(heap, 8);
    pushMinHeap(heap, 3);
    printf("   堆大小=%d, 堆顶(最小值)=%d\n", size(heap), peekMinHeap(heap));

    printf("3. 依次出堆: ");
    while (!isEmpty(heap)) {
        printf("%d ", popMinHeap(heap));
    }
    printf("\n");
    delMaxHeap(heap);

    printf("4. 查找数组 {3,1,5,12,2,11} 中最大的 3 个元素\n");
    int nums[] = {3, 1, 5, 12, 2, 11};
    int n;
    int *res = topKHeap(nums, 6, 3);
    n = 3;
    printf("   结果（堆数组顺序，非排序）: ");
    printSeq(res, n);
    free(res);
    return 0;
}

