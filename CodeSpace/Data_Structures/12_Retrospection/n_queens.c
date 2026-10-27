#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE 100

/* 回溯算法：n 皇后 */
void backtrack(int row, int n, char state[MAX_SIZE][MAX_SIZE], char ***res, int *resSize, bool cols[MAX_SIZE],
               bool diags1[2 * MAX_SIZE - 1], bool diags2[2 * MAX_SIZE - 1]) {
    // 当放置完所有行时，记录解
    if (row == n) {
        res[*resSize] = (char **)malloc(sizeof(char *) * n);
        for (int i = 0; i < n; ++i) {
            res[*resSize][i] = (char *)malloc(sizeof(char) * (n + 1));
            memcpy(res[*resSize][i], state[i], n + 1);
        }
        (*resSize)++;
        return;
    }
    // 遍历所有列
    for (int col = 0; col < n; col++) {
        // 计算该格子对应的主对角线和次对角线
        int diag1 = row - col + n - 1;
        int diag2 = row + col;
        // 剪枝：不允许该格子所在列、主对角线、次对角线上存在皇后
        if (!cols[col] && !diags1[diag1] && !diags2[diag2]) {
            // 尝试：将皇后放置在该格子
            state[row][col] = 'Q';
            cols[col] = diags1[diag1] = diags2[diag2] = true;
            // 放置下一行
            backtrack(row + 1, n, state, res, resSize, cols, diags1, diags2);
            // 回退：将该格子恢复为空位
            state[row][col] = '#';
            cols[col] = diags1[diag1] = diags2[diag2] = false;
        }
    }
}

/* 求解 n 皇后 */
char ***nQueens(int n, int *returnSize) {
    char state[MAX_SIZE][MAX_SIZE];
    // 初始化 n*n 大小的棋盘，其中 'Q' 代表皇后，'#' 代表空位
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            state[i][j] = '#';
        }
        state[i][n] = '\0';
    }
    bool cols[MAX_SIZE] = {false};           // 记录列是否有皇后
    bool diags1[2 * MAX_SIZE - 1] = {false}; // 记录主对角线上是否有皇后
    bool diags2[2 * MAX_SIZE - 1] = {false}; // 记录次对角线上是否有皇后

    char ***res = (char ***)malloc(sizeof(char **) * MAX_SIZE);
    *returnSize = 0;
    backtrack(0, n, state, res, returnSize, cols, diags1, diags2);
    return res;
}

int main(void) {
    int n = 4, count = 0;
    char ***res = nQueens(n, &count);
    printf("%d 皇后共有 %d 种解法\n", n, count);
    for (int k = 0; k < count; k++) {
        printf("解法 %d:\n", k + 1);
        for (int i = 0; i < n; i++) {
            printf("  %s\n", res[k][i]);
            free(res[k][i]);
        }
        free(res[k]);
    }
    free(res);
    return 0;
}
