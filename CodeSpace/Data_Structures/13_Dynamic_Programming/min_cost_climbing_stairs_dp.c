#include <stdio.h>
#include <stdlib.h>

#define myMin(a, b) ((a) < (b) ? (a) : (b))

/* 爬楼梯最小代价：动态规划 */
int minCostClimbingStairsDP(int cost[], int costSize) {
    int n = costSize - 1;
    if (n == 1 || n == 2)
        return cost[n];
    // 初始化 dp 表，用于存储子问题的解
    int *dp = calloc(n + 1, sizeof(int));
    // 初始状态：预设最小子问题的解
    dp[1] = cost[1];
    dp[2] = cost[2];
    // 状态转移：从较小子问题逐步求解较大子问题
    for (int i = 3; i <= n; i++) {
        dp[i] = myMin(dp[i - 1], dp[i - 2]) + cost[i];
    }
    int res = dp[n];
    // 释放内存
    free(dp);
    return res;
}

/* 爬楼梯最小代价：空间优化后的动态规划 */
int minCostClimbingStairsDPComp(int cost[], int costSize) {
    int n = costSize - 1;
    if (n == 1 || n == 2)
        return cost[n];
    int a = cost[1], b = cost[2];
    for (int i = 3; i <= n; i++) {
        int tmp = b;
        b = myMin(a, tmp) + cost[i];
        a = tmp;
    }
    return b;
}

/* 带约束爬楼梯：动态规划 */
int climbingStairsConstraintDP(int n) {
    if (n == 1 || n == 2) {
        return 1;
    }
    // 初始化 dp 表，用于存储子问题的解
    int **dp = malloc((n + 1) * sizeof(int *));
    for (int i = 0; i <= n; i++) {
        dp[i] = calloc(3, sizeof(int));
    }
    // 初始状态：预设最小子问题的解
    dp[1][1] = 1;
    dp[1][2] = 0;
    dp[2][1] = 0;
    dp[2][2] = 1;
    // 状态转移：从较小子问题逐步求解较大子问题
    for (int i = 3; i <= n; i++) {
        dp[i][1] = dp[i - 1][2];
        dp[i][2] = dp[i - 2][1] + dp[i - 2][2];
    }
    int res = dp[n][1] + dp[n][2];
    // 释放内存
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
    }
    free(dp);
    return res;
}

int main(void) {
    int cost[] = {0, 1, 100, 1, 1, 1, 100, 1, 1, 100, 1};
    int size = 11;
    printf("每阶代价: ");
    for (int i = 0; i < size; i++) printf("%d ", cost[i]);
    printf("\n最小代价（动态规划）: %d\n", minCostClimbingStairsDP(cost, size));
    printf("最小代价（空间优化）: %d\n", minCostClimbingStairsDPComp(cost, size));
    printf("带约束爬楼梯（不能连续爬 1 阶）:\n");
    for (int n = 1; n <= 8; n++) {
        printf("  %d 阶: %d 种方案\n", n, climbingStairsConstraintDP(n));
    }
    return 0;
}
