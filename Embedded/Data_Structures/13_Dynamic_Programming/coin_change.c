#include <stdio.h>
#include <stdlib.h>

#define myMin(a, b) ((a) < (b) ? (a) : (b))

/* 零钱兑换：动态规划 */
int coinChangeDP(int coins[], int amt, int coinsSize) {
    int n = coinsSize;
    int MAX = amt + 1;
    // 初始化 dp 表
    int **dp = malloc((n + 1) * sizeof(int *));
    for (int i = 0; i <= n; i++) {
        dp[i] = calloc(amt + 1, sizeof(int));
    }
    // 状态转移：首行首列
    for (int a = 1; a <= amt; a++) {
        dp[0][a] = MAX;
    }
    // 状态转移：其余行和列
    for (int i = 1; i <= n; i++) {
        for (int a = 1; a <= amt; a++) {
            if (coins[i - 1] > a) {
                // 若超过目标金额，则不选硬币 i
                dp[i][a] = dp[i - 1][a];
            } else {
                // 不选和选硬币 i 这两种方案的较小值
                dp[i][a] = myMin(dp[i - 1][a], dp[i][a - coins[i - 1]] + 1);
            }
        }
    }
    int res = dp[n][amt] != MAX ? dp[n][amt] : -1;
    // 释放内存
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
    }
    free(dp);
    return res;
}

/* 零钱兑换：空间优化后的动态规划 */
int coinChangeDPComp(int coins[], int amt, int coinsSize) {
    int n = coinsSize;
    int MAX = amt + 1;
    // 初始化 dp 表
    int *dp = malloc((amt + 1) * sizeof(int));
    for (int j = 1; j <= amt; j++) {
        dp[j] = MAX;
    }
    dp[0] = 0;

    // 状态转移
    for (int i = 1; i <= n; i++) {
        for (int a = 1; a <= amt; a++) {
            if (coins[i - 1] > a) {
                // 若超过目标金额，则不选硬币 i
                dp[a] = dp[a];
            } else {
                // 不选和选硬币 i 这两种方案的较小值
                dp[a] = myMin(dp[a], dp[a - coins[i - 1]] + 1);
            }
        }
    }
    int res = dp[amt] != MAX ? dp[amt] : -1;
    // 释放内存
    free(dp);
    return res;
}

int main(void) {
    int coins[] = {1, 2, 5};
    printf("硬币 1 2 5，凑出 11 的最少硬币数\n");
    printf("动态规划: %d\n", coinChangeDP(coins, 11, 3));
    printf("空间优化: %d\n", coinChangeDPComp(coins, 11, 3));
    int coins2[] = {2};
    printf("硬币 2，凑出 3 -> %d (-1 表示无解)\n", coinChangeDP(coins2, 3, 1));
    return 0;
}


