#include <stdio.h>

/* 搜索 */
int dfs(int i) {
    // 已知 dp[1] 和 dp[2] ，返回之
    if (i == 1 || i == 2)
        return i;
    // dp[i] = dp[i-1] + dp[i-2]
    int count = dfs(i - 1) + dfs(i - 2);
    return count;
}

/* 爬楼梯：搜索 */
int climbingStairsDFS(int n) { return dfs(n); }

int main(void) {
    for (int n = 1; n <= 10; n++) {
        printf("爬 %2d 阶楼梯（暴力搜索）: %d 种方案\n", n, climbingStairsDFS(n));
    }
    return 0;
}
