#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_SIZE 100

/* 回溯算法：全排列 I */
void backtrack(int *state, int stateSize, int *choices, int choicesSize, bool *selected, int **res, int *resSize) {
    // 当状态长度等于元素数量时，记录解
    if (stateSize == choicesSize) {
        res[*resSize] = (int *)malloc(choicesSize * sizeof(int));
        for (int i = 0; i < choicesSize; i++) {
            res[*resSize][i] = state[i];
        }
        (*resSize)++;
        return;
    }
    // 遍历所有选择
    for (int i = 0; i < choicesSize; i++) {
        int choice = choices[i];
        // 剪枝：不允许重复选择元素
        if (!selected[i]) {
            // 尝试：做出选择，更新状态
            selected[i] = true;
            state[stateSize] = choice;
            // 进行下一轮选择
            backtrack(state, stateSize + 1, choices, choicesSize, selected, res, resSize);
            // 回退：撤销选择，恢复到之前的状态
            selected[i] = false;
        }
    }
}

/* 全排列 I */
int **permutationsI(int *nums, int numsSize, int *returnSize) {
    int *state = (int *)malloc(numsSize * sizeof(int));
    bool *selected = (bool *)malloc(numsSize * sizeof(bool));
    for (int i = 0; i < numsSize; i++) {
        selected[i] = false;
    }
    int **res = (int **)malloc(MAX_SIZE * sizeof(int *));
    *returnSize = 0;

    backtrack(state, 0, nums, numsSize, selected, res, returnSize);

    free(state);
    free(selected);

    return res;
}

int main(void) {
    int nums[] = {1, 2, 3};
    int count = 0;
    int **res = permutationsI(nums, 3, &count);
    printf("输入: 1 2 3，共 %d 种全排列\n", count);
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 3; j++) printf("%d ", res[i][j]);
        printf("\n");
        free(res[i]);
    }
    free(res);
    return 0;
}
