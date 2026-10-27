#include <stdio.h>

/* 二分查找：问题 f(i, j) */
int dfs(int nums[], int target, int i, int j) {
    // 若区间为空，代表无目标元素，则返回 -1
    if (i > j) {
        return -1;
    }
    // 计算中点索引 m
    int m = (i + j) / 2;
    if (nums[m] < target) {
        // 递归子问题 f(m+1, j)
        return dfs(nums, target, m + 1, j);
    } else if (nums[m] > target) {
        // 递归子问题 f(i, m-1)
        return dfs(nums, target, i, m - 1);
    } else {
        // 找到目标元素，返回其索引
        return m;
    }
}

/* 二分查找 */
int binarySearch(int nums[], int target, int numsSize) {
    int n = numsSize;
    // 求解问题 f(0, n-1)
    return dfs(nums, target, 0, n - 1);
}

int main(void) {
    int nums[] = {1, 3, 6, 8, 12, 15, 23};
    int n = 7;
    printf("数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n查找 12 -> 下标 %d\n", binarySearch(nums, 12, n));
    printf("查找 7  -> 下标 %d (-1 表示不存在)\n", binarySearch(nums, 7, n));
    return 0;
}
