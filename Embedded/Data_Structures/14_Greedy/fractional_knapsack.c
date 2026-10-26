#include <stdio.h>
#include <stdlib.h>

/* 物品 */
typedef struct {
    int w; // 物品重量
    int v; // 物品价值
} Item;

/* 按单位价值从高到低排序 */
int sortByValueDensity(const void *a, const void *b) {
    const Item *x = (const Item *)a;
    const Item *y = (const Item *)b;
    // 比较 x.v / x.w 与 y.v / y.w，避免浮点除法
    long lhs = (long)x->v * y->w;
    long rhs = (long)y->v * x->w;
    return (lhs < rhs) - (lhs > rhs);
}

/* 分数背包：贪心 */
float fractionalKnapsack(int wgt[], int val[], int itemCount, int cap) {
    // 创建物品列表，包含两个属性：重量、价值
    Item *items = malloc(sizeof(Item) * itemCount);
    for (int i = 0; i < itemCount; i++) {
        items[i] = (Item){.w = wgt[i], .v = val[i]};
    }
    // 按照单位价值 item.v / item.w 从高到低进行排序
    qsort(items, (size_t)itemCount, sizeof(Item), sortByValueDensity);
    // 循环贪心选择
    float res = 0.0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].w <= cap) {
            // 若剩余容量充足，则将当前物品整个装进背包
            res += items[i].v;
            cap -= items[i].w;
        } else {
            // 若剩余容量不足，则将当前物品的一部分装进背包
            res += (float)cap / items[i].w * items[i].v;
            cap = 0;
            break;
        }
    }
    free(items);
    return res;
}

int main(void) {
    int wgt[] = {10, 20, 30, 40, 50};
    int val[] = {50, 120, 150, 210, 240};
    int cap = 50;
    printf("物品重量: 10 20 30 40 50\n物品价值: 50 120 150 210 240\n背包容量: %d\n", cap);
    printf("分数背包最大价值: %.1f\n", fractionalKnapsack(wgt, val, 5, cap));
    return 0;
}
