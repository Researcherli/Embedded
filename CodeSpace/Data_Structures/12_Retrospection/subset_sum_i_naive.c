#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

int state[MAX_SIZE];                // 当前子集
int stateSize = 0;
int res[MAX_SIZE][MAX_SIZE];        // 所有解
int resColSizes[MAX_SIZE];          // 每个解的长度
int resSize = 0;

/* 回溯算法：子集和 I */
void backtrack(int target, int total, int *choices, int choicesSize) {
    // 子集和等于 target 时，记录解
    if (total == target) {
        for (int i = 0; i < stateSize; i++) {
            res[resSize][i] = state[i];
        }
        resColSizes[resSize++] = stateSize;
        return;
    }
    // 遍历所有选择
    for (int i = 0; i < choicesSize; i++) {
        // 剪枝：若子集和超过 target ，则跳过该选择
        if (total + choices[i] > target) {
            continue;
        }
        // 尝试：做出选择，更新元素和 total
        state[stateSize++] = choices[i];
        // 进行下一轮选择
        backtrack(target, total + choices[i], choices, choicesSize);
        // 回退：撤销选择，恢复到之前的状态
        stateSize--;
    }
}

/* 求解子集和 I（包含重复子集） */
void subsetSumINaive(int *nums, int numsSize, int target) {
    resSize = 0; // 初始化解的数量为0
    backtrack(target, 0, nums, numsSize);
}

int main(void) {
    int nums[] = {3, 4, 5};
    int target = 9;
    subsetSumINaive(nums, 3, target);
    printf("nums = 3 4 5, target = %d，共 %d 个解（含重复子集）\n", target, resSize);
    for (int i = 0; i < resSize; i++) {
        printf("  ");
        for (int j = 0; j < resColSizes[i]; j++) printf("%d ", res[i][j]);
        printf("\n");
    }
    return 0;
}
