#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 获取元素 num 的第 k 位，其中 exp = 10^(k-1) */
int digit(int num, int exp) {
    // 传入 exp 而非 k 可以避免在此重复执行昂贵的次方计算
    return (num / exp) % 10;
}

/* 计数排序（根据 nums 第 k 位排序） */
void countingSortDigit(int nums[], int size, int exp) {
    // 十进制的位范围为 0~9 ，因此需要长度为 10 的桶数组
    int *counter = (int *)malloc((sizeof(int) * 10));
    memset(counter, 0, sizeof(int) * 10); // 初始化为 0 以支持后续内存释放
    // 统计 0~9 各数字的出现次数
    for (int i = 0; i < size; i++) {
        // 获取 nums[i] 第 k 位，记为 d
        int d = digit(nums[i], exp);
        // 统计数字 d 的出现次数
        counter[d]++;
    }
    // 求前缀和，将“出现个数”转换为“数组索引”
    for (int i = 1; i < 10; i++) {
        counter[i] += counter[i - 1];
    }
    // 倒序遍历，根据桶内统计结果，将各元素填入 res
    int *res = (int *)malloc(sizeof(int) * size);
    for (int i = size - 1; i >= 0; i--) {
        int d = digit(nums[i], exp);
        int j = counter[d] - 1; // 获取 d 在数组中的索引 j
        res[j] = nums[i];       // 将当前元素填入索引 j
        counter[d]--;           // 将 d 的数量减 1
    }
    // 使用结果覆盖原数组 nums
    for (int i = 0; i < size; i++) {
        nums[i] = res[i];
    }
    // 释放内存
    free(res);
    free(counter);
}

/* 基数排序 */
void radixSort(int nums[], int size) {
    // 获取数组的最大元素，用于判断最大位数
    int max = INT32_MIN;
    for (int i = 0; i < size; i++) {
        if (nums[i] > max) {
            max = nums[i];
        }
    }
    // 按照从低位到高位的顺序遍历
    for (int exp = 1; max >= exp; exp *= 10)
        // 对数组元素的第 k 位执行计数排序
        // k = 1 -> exp = 1
        // k = 2 -> exp = 10
        // 即 exp = 10^(k-1)
        countingSortDigit(nums, size, exp);
}

int main(void) {
    int nums[] = {170, 45, 75, 90, 802, 24, 2, 66};
    int n = 8;
    printf("原数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    radixSort(nums, n);
    printf("\n基数排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n");
    return 0;
}


