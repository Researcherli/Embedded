/*
 * @lc app=leetcode.cn id=509 lang=c
 *
 * [509] 斐波那契数
 */

// @lc code=start
int fib(int n) {
    if (n == 0 || n == 1) {
        return n;
    }
    int a = 0, b = 1, sum;
    for (int i = 2; i <= n; i++) {
        sum = a + b;
        a = b;
        b = sum;
    }
    return sum;
}
// @lc code=end
