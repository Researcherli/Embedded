#include <stdio.h>

#define BINARY_SEARCH_INSERTION_NO_MAIN
#include "binary_search_insertion.c"

/* 二分查找最左一个 target */
int binarySearchLeftEdge(int *nums, int numSize, int target) {
    // 等价于查找 target 的插入点
    int i = binarySearchInsertion(nums, numSize, target);
    // 未找到 target ，返回 -1
    if (i == numSize || nums[i] != target) {
        return -1;
    }
    // 找到 target ，返回索引 i
    return i;
}

/* 二分查找最右一个 target */
int binarySearchRightEdge(int *nums, int numSize, int target) {
    // 转化为查找最左一个 target + 1
    int i = binarySearchInsertion(nums, numSize, target + 1);
    // j 指向最右一个 target ，i 指向首个大于 target 的元素
    int j = i - 1;
    // 未找到 target ，返回 -1
    if (j == -1 || nums[j] != target) {
        return -1;
    }
    // 找到 target ，返回索引 j
    return j;
}

int main(void) {
    int nums[] = {1, 2, 2, 2, 3, 4, 5, 5};
    printf("数组 {1,2,2,2,3,4,5,5}\n");
    printf("1. 最左一个 2 的索引: %d\n", binarySearchLeftEdge(nums, 8, 2));
    printf("2. 最右一个 2 的索引: %d\n", binarySearchRightEdge(nums, 8, 2));
    printf("3. 最左一个 5 的索引: %d\n", binarySearchLeftEdge(nums, 8, 5));
    printf("4. 最右一个 5 的索引: %d\n", binarySearchRightEdge(nums, 8, 5));
    printf("5. 不存在的 6: 左=%d, 右=%d\n",
           binarySearchLeftEdge(nums, 8, 6), binarySearchRightEdge(nums, 8, 6));
    return 0;
}

