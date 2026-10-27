#include <stdio.h>

/* 选择排序 */
void selectionSort(int nums[], int n) {
    // 外循环：未排序区间为 [i, n-1]
    for (int i = 0; i < n - 1; i++) {
        // 内循环：找到未排序区间内的最小元素
        int k = i;
        for (int j = i + 1; j < n; j++) {
            if (nums[j] < nums[k])
                k = j; // 记录最小元素的索引
        }
        // 将该最小元素与未排序区间的首个元素交换
        int temp = nums[i];
        nums[i] = nums[k];
        nums[k] = temp;
    }
}

int main(void) {
    int nums[] = {5, 2, 9, 1, 5, 6};
    int n = 6;
    printf("原数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    selectionSort(nums, n);
    printf("\n选择排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n");
    return 0;
}
