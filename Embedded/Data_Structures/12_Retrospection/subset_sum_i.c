#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

int state[MAX_SIZE];                // 当前子集
int stateSize = 0;
int res[MAX_SIZE][MAX_SIZE];        // 所有解
int resColSizes[MAX_SIZE];          // 每个解的长度
int resSize = 0;

int cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

/* 回溯算法：子集和 I */
void backtrack(int target, int *choices, int choicesSize, int start) {
    // 子集和等于 target 时，记录解
    if (target == 0) {
        for (int i = 0; i < stateSize; ++i) {
            res[resSize][i] = state[i];
        }
        resColSizes[resSize++] = stateSize;
        return;
    }
    // 遍历所有选择
    // 剪枝二：从 start 开始遍历，避免生成重复子集
    for (int i = start; i < choicesSize; i++) {
        // 剪枝一：若子集和超过 target ，则直接结束循环
        // 这是因为数组已排序，后边元素更大，子集和一定超过 target
        if (target - choices[i] < 0) {
            break;
        }
        // 尝试：做出选择，更新 target, start
        state[stateSize] = choices[i];
        stateSize++;
        // 进行下一轮选择
        backtrack(target - choices[i], choices, choicesSize, i);
        // 回退：撤销选择，恢复到之前的状态
        stateSize--;
    }
}

/* 求解子集和 I */
void subsetSumI(int *nums, int numsSize, int target) {
    qsort(nums, numsSize, sizeof(int), cmp); // 对 nums 进行排序
    int start = 0;                           // 遍历起始点
    backtrack(target, nums, numsSize, start);
}

int main(void) {
    int nums[] = {4, 3, 5};
    int target = 9;
    subsetSumI(nums, 3, target);
    printf("nums = 4 3 5（先排序），target = %d，共 %d 个解\n", target, resSize);
    for (int i = 0; i < resSize; i++) {
        printf("  ");
        for (int j = 0; j < resColSizes[i]; j++) printf("%d ", res[i][j]);
        printf("\n");
    }
    return 0;
}
