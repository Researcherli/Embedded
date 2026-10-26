#include <stdio.h>
#include <stdlib.h>

static int compare(const void *a, const void *b) {
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

/* 桶排序（输入元素需在 [0, 1) 区间内） */
void bucketSort(float nums[], int n) {
    int k = n;                                     // 初始化 k = n 个桶，保证索引 idx 在 [0, k-1] 内
    int *sizes = malloc(k * sizeof(int));          // 记录每个桶的大小
    float **buckets = malloc(k * sizeof(float *)); // 动态数组的数组（桶）
    // 为每个桶预分配足够的空间
    for (int i = 0; i < k; ++i) {
        buckets[i] = (float *)malloc(n * sizeof(float));
        sizes[i] = 0;
    }
    // 1. 将数组元素分配到各个桶中
    for (int i = 0; i < n; ++i) {
        int idx = (int)(nums[i] * k);
        buckets[idx][sizes[idx]++] = nums[i];
    }
    // 2. 对各个桶执行排序
    for (int i = 0; i < k; ++i) {
        qsort(buckets[i], sizes[i], sizeof(float), compare);
    }
    // 3. 合并排序后的桶
    int idx = 0;
    for (int i = 0; i < k; ++i) {
        for (int j = 0; j < sizes[i]; ++j) {
            nums[idx++] = buckets[i][j];
        }
        // 释放内存
        free(buckets[i]);
    }
    free(sizes);
    free(buckets);
}

int main(void) {
    float nums[] = {0.78f, 0.17f, 0.39f, 0.26f, 0.72f, 0.94f, 0.21f, 0.12f, 0.23f, 0.68f};
    int n = 10;
    printf("原数组: ");
    for (int i = 0; i < n; i++) printf("%.2f ", nums[i]);
    bucketSort(nums, n);
    printf("\n桶排序后: ");
    for (int i = 0; i < n; i++) printf("%.2f ", nums[i]);
    printf("\n");
    return 0;
}
