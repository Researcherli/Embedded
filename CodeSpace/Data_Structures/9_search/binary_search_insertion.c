#include <stdio.h>

/* 二分查找插入点（无重复元素） */
int binarySearchInsertionSimple(int *nums, int numSize, int target) {
    int i = 0, j = numSize - 1; // 初始化双闭区间 [0, n-1]
    while (i <= j) {
        int m = i + (j - i) / 2; // 计算中点索引 m
        if (nums[m] < target) {
            i = m + 1; // target 在区间 [m+1, j] 中
        } else if (nums[m] > target) {
            j = m - 1; // target 在区间 [i, m-1] 中
        } else {
            return m; // 找到 target ，返回插入点 m
        }
    }
    // 未找到 target ，返回插入点 i
    return i;
}

/* 二分查找插入点（存在重复元素） */
int binarySearchInsertion(int *nums, int numSize, int target) {
    int i = 0, j = numSize - 1; // 初始化双闭区间 [0, n-1]
    while (i <= j) {
        int m = i + (j - i) / 2; // 计算中点索引 m
        if (nums[m] < target) {
            i = m + 1; // target 在区间 [m+1, j] 中
        } else if (nums[m] > target) {
            j = m - 1; // target 在区间 [i, m-1] 中
        } else {
            j = m - 1; // 最右一个小于 target 的元素在区间 [i, m-1] 中
        }
    }
    // 返回插入点 i
    return i;
}

#ifndef BINARY_SEARCH_INSERTION_NO_MAIN
int main(void) {
    int nums[] = {1, 3, 5, 7, 9, 11};
    printf("1. 无重复数组 {1,3,5,7,9,11}\n");
    printf("   binarySearchInsertionSimple(7) = %d (找到，返回其索引)\n",
           binarySearchInsertionSimple(nums, 6, 7));
    printf("   binarySearchInsertionSimple(8) = %d (未找到，返回插入点)\n",
           binarySearchInsertionSimple(nums, 6, 8));

    int dup[] = {1, 3, 6, 6, 6, 9};
    printf("2. 有重复数组 {1,3,6,6,6,9}\n");
    printf("   binarySearchInsertion(6) = %d (最左 6 的位置)\n",
           binarySearchInsertion(dup, 6, 6));
    printf("   binarySearchInsertion(7) = %d (插入点)\n",
           binarySearchInsertion(dup, 6, 7));
    return 0;
}
#endif


