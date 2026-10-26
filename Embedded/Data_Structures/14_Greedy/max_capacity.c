#include <stdio.h>

#define myMin(a, b) ((a) < (b) ? (a) : (b))
#define myMax(a, b) ((a) > (b) ? (a) : (b))

/* 最大容量：贪心 */
int maxCapacity(int ht[], int htLength) {
    // 初始化 i, j，使其分列数组两端
    int i = 0;
    int j = htLength - 1;
    // 初始最大容量为 0
    int res = 0;
    // 循环贪心选择，直至两板相遇
    while (i < j) {
        // 更新最大容量
        int capacity = myMin(ht[i], ht[j]) * (j - i);
        res = myMax(res, capacity);
        // 向内移动短板
        if (ht[i] < ht[j]) {
            i++;
        } else {
            j--;
        }
    }
    return res;
}

int main(void) {
    int ht[] = {3, 8, 5, 2, 7, 6, 4};
    int n = 7;
    printf("板高: ");
    for (int i = 0; i < n; i++) printf("%d ", ht[i]);
    printf("\n最大容量: %d\n", maxCapacity(ht, n));
    return 0;
}
