#include <stdio.h>

/* 插入排序 */
void insertionSort(int nums[], int size) {
    // 外循环：已排序区间为 [0, i-1]
    for (int i = 1; i < size; i++) {
        int base = nums[i], j = i - 1;
        // 内循环：将 base 插入到已排序区间 [0, i-1] 中的正确位置
        while (j >= 0 && nums[j] > base) {
            // 将 nums[j] 向右移动一位
            nums[j + 1] = nums[j];
            j--;
        }
        // 将 base 赋值到正确位置
        nums[j + 1] = base;
    }
}

int main(void) {
    int nums[] = {12, 11, 13, 5, 6};
    int n = 5;
    printf("原数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    insertionSort(nums, n);
    printf("\n插入排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n");
    return 0;
}
