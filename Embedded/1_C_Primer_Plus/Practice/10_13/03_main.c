#include <stdio.h>

int find_max(const int *arr, size_t size)
{
    // 防御性检查：避免空数组解引用
    if (size == 0)
    {
        return 0; // 或者使用特定错误码
    }

    // 核心修正：以首元素作为初始最大值，避免负数数组误判
    int max_val = arr[0];

    // 从索引 1 开始向后对比
    for (size_t i = 1; i < size; i++)
    {
        if (arr[i] > max_val)
        {
            max_val = arr[i];
        }
    }
    return max_val;
}

int main(void)
{
    int source[5] = {1, 2, 6, 4, 5};
    int negative_test[4] = {-10, -3, -5, -8};

    printf("source max = %d\n", find_max(source, 5));
    printf("negative_test max = %d\n", find_max(negative_test, 4)); // 正确输出 -3

    return 0;
}
