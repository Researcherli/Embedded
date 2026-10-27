#include <stdio.h>

/* 零钱兑换：贪心 */
int coinChangeGreedy(int *coins, int size, int amt) {
    // 假设 coins 列表有序
    int i = size - 1;
    int count = 0;
    // 循环进行贪心选择，直到无剩余金额
    while (amt > 0) {
        // 找到小于且最接近剩余金额的硬币
        while (i > 0 && coins[i] > amt) {
            i--;
        }
        // 选择 coins[i]
        amt -= coins[i];
        count++;
    }
    // 若未找到可行方案，则返回 -1
    return amt == 0 ? count : -1;
}

int main(void) {
    int coins1[] = {1, 5, 10, 20, 50, 100};
    printf("硬币 1 5 10 20 50 100，凑 187 -> 贪心需要 %d 枚\n", coinChangeGreedy(coins1, 6, 187));
    int coins2[] = {1, 20, 50};
    printf("硬币 1 20 50，凑 60 -> 贪心需要 %d 枚（最优解是 3 枚 20，贪心并非总是最优）\n",
           coinChangeGreedy(coins2, 3, 60));
    return 0;
}


