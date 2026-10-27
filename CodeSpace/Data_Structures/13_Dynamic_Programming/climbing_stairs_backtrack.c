#include <stdio.h>
#include <stdlib.h>

/* 回溯 */
void backtrack(int *choices, int state, int n, int *res, int len) {
    // 当爬到第 n 阶时，方案数量加 1
    if (state == n)
        res[0]++;
    // 遍历所有选择
    for (int i = 0; i < len; i++) {
        int choice = choices[i];
        // 剪枝：不允许越过第 n 阶
        if (state + choice > n)
            continue;
        // 尝试：做出选择，更新状态
        backtrack(choices, state + choice, n, res, len);
        // 回退
    }
}

/* 爬楼梯：回溯 */
int climbingStairsBacktrack(int n) {
    int choices[2] = {1, 2}; // 可选择向上爬 1 阶或 2 阶
    int state = 0;           // 从第 0 阶开始爬
    int *res = (int *)malloc(sizeof(int));
    *res = 0; // 使用 res[0] 记录方案数量
    int len = sizeof(choices) / sizeof(int);
    backtrack(choices, state, n, res, len);
    int result = *res;
    free(res);
    return result;
}

int main(void) {
    for (int n = 1; n <= 6; n++) {
        printf("爬 %d 阶楼梯: %d 种方案\n", n, climbingStairsBacktrack(n));
    }
    return 0;
}
