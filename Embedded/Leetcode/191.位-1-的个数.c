/*
 * @lc app=leetcode.cn id=191 lang=c
 *
 * [191] 位1的个数
 */

// @lc code=start
int hammingWeight(int n) {
    int i = 0; // 表示1的个数
    while (n > 0) {
        i += n & 1;
        n >>= 1;
    }
    return i;
}

// int hammingWeight(int n) {
//     int i = 0; // 表示1的个数
//     while (n != 0) {
//         i++;
//         n &= (n - 1);
//     }
//     return i;
// }
// @lc code=end
