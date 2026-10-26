#include <stdbool.h>
#include <stdio.h>

/* 冒泡排序 */
void bubbleSort(int nums[], int size) {
    // 外循环：未排序区间为 [0, i]
    for (int i = size - 1; i > 0; i--) {
        // 内循环：将未排序区间 [0, i] 中的最大元素交换至该区间的最右端
        for (int j = 0; j < i; j++) {
            if (nums[j] > nums[j + 1]) {
                int temp = nums[j];
                nums[j] = nums[j + 1];
                nums[j + 1] = temp;
            }
        }
    }
}

/* 冒泡排序（标志优化）*/
void bubbleSortWithFlag(int nums[], int size) {
    // 外循环：未排序区间为 [0, i]
    for (int i = size - 1; i > 0; i--) {
        bool flag = false;
        // 内循环：将未排序区间 [0, i] 中的最大元素交换至该区间的最右端
        for (int j = 0; j < i; j++) {
            if (nums[j] > nums[j + 1]) {
                int temp = nums[j];
                nums[j] = nums[j + 1];
                nums[j + 1] = temp;
                flag = true;
            }
        }
        if (!flag)
            break;
    }
}

int main(void) {
    int nums[] = {64, 34, 25, 12, 22, 11, 90};
    int nums2[] = {64, 34, 25, 12, 22, 11, 90};
    int n = 7;
    printf("原数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    bubbleSort(nums, n);
    printf("\n冒泡排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    bubbleSortWithFlag(nums2, n);
    printf("\n带标志冒泡排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", nums2[i]);
    printf("\n");
    return 0;
}
