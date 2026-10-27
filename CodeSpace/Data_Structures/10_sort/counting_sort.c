#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 计数排序 */
// 简单实现，无法用于排序对象
void countingSortNaive(int nums[], int size) {
    // 1. 统计数组最大元素 m
    int m = 0;
    for (int i = 0; i < size; i++) {
        if (nums[i] > m) {
            m = nums[i];
        }
    }
    // 2. 统计各数字的出现次数
    // counter[num] 代表 num 的出现次数
    int *counter = calloc(m + 1, sizeof(int));
    for (int i = 0; i < size; i++) {
        counter[nums[i]]++;
    }
    // 3. 遍历 counter ，将各元素填入原数组 nums
    int i = 0;
    for (int num = 0; num < m + 1; num++) {
        for (int j = 0; j < counter[num]; j++, i++) {
            nums[i] = num;
        }
    }
    // 4. 释放内存
    free(counter);
}

/* 计数排序 */
// 完整实现，可排序对象，并且是稳定排序
void countingSort(int nums[], int size) {
    // 1. 统计数组最大元素 m
    int m = 0;
    for (int i = 0; i < size; i++) {
        if (nums[i] > m) {
            m = nums[i];
        }
    }
    // 2. 统计各数字的出现次数
    // counter[num] 代表 num 的出现次数
    int *counter = calloc(m + 1, sizeof(int));
    for (int i = 0; i < size; i++) {
        counter[nums[i]]++;
    }
    // 3. 求 counter 的前缀和，将“出现次数”转换为“尾索引”
    // 即 counter[num]-1 是 num 在 res 中最后一次出现的索引
    for (int i = 0; i < m; i++) {
        counter[i + 1] += counter[i];
    }
    // 4. 倒序遍历 nums ，将各元素填入结果数组 res
    // 初始化数组 res 用于记录结果
    int *res = malloc(sizeof(int) * size);
    for (int i = size - 1; i >= 0; i--) {
        int num = nums[i];
        res[counter[num] - 1] = num; // 将 num 放置到对应索引处
        counter[num]--;              // 令前缀和自减 1 ，得到下次放置 num 的索引
    }
    // 使用结果数组 res 覆盖原数组 nums
    memcpy(nums, res, size * sizeof(int));
    // 5. 释放内存
    free(res);
    free(counter);
}

int main(void) {
    int nums[] = {1, 4, 1, 2, 7, 5, 2};
    int n = 7;
    int nums2[] = {1, 4, 1, 2, 7, 5, 2};
    printf("原数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    countingSortNaive(nums, n);
    printf("\n朴素计数排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    countingSort(nums2, n);
    printf("\n完整计数排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", nums2[i]);
    printf("\n");
    return 0;
}
