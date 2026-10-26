#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define myMin(a, b) ((a) < (b) ? (a) : (b))

/* 编辑距离：动态规划 */
int editDistanceDP(char *s, char *t, int n, int m) {
    int **dp = malloc((n + 1) * sizeof(int *));
    for (int i = 0; i <= n; i++) {
        dp[i] = calloc(m + 1, sizeof(int));
    }
    // 状态转移：首行首列
    for (int i = 1; i <= n; i++) {
        dp[i][0] = i;
    }
    for (int j = 1; j <= m; j++) {
        dp[0][j] = j;
    }
    // 状态转移：其余行和列
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (s[i - 1] == t[j - 1]) {
                // 若两字符相等，则直接跳过此两字符
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                // 最少编辑步数 = 插入、删除、替换这三种操作的最少编辑步数 + 1
                dp[i][j] = myMin(myMin(dp[i][j - 1], dp[i - 1][j]), dp[i - 1][j - 1]) + 1;
            }
        }
    }
    int res = dp[n][m];
    // 释放内存
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
    }
    return res;
}
/* 编辑距离：空间优化后的动态规划 */
int editDistanceDPComp(char *s, char *t, int n, int m) {
    int *dp = calloc(m + 1, sizeof(int));
    // 状态转移：首行
    for (int j = 1; j <= m; j++) {
        dp[j] = j;
    }
    // 状态转移：其余行
    for (int i = 1; i <= n; i++) {
        // 状态转移：首列
        int leftup = dp[0]; // 暂存 dp[i-1, j-1]
        dp[0] = i;
        // 状态转移：其余列
        for (int j = 1; j <= m; j++) {
            int temp = dp[j];
            if (s[i - 1] == t[j - 1]) {
                // 若两字符相等，则直接跳过此两字符
                dp[j] = leftup;
            } else {
                // 最少编辑步数 = 插入、删除、替换这三种操作的最少编辑步数 + 1
                dp[j] = myMin(myMin(dp[j - 1], dp[j]), leftup) + 1;
            }
            leftup = temp; // 更新为下一轮的 dp[i-1, j-1]
        }
    }
    int res = dp[m];
    // 释放内存
    free(dp);
    return res;
}

int main(void) {
    char *s = "kitten", *t = "sitting";
    int n = (int)strlen(s), m = (int)strlen(t);
    printf("把 \"%s\" 变成 \"%s\" 的最少编辑步数\n", s, t);
    printf("动态规划: %d\n", editDistanceDP(s, t, n, m));
    printf("空间优化: %d\n", editDistanceDPComp(s, t, n, m));
    return 0;
}

