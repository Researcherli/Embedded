#include <stdio.h>

// 计算 double 数组中最大值与最小值的差值
double get_array_range(const double arr[], size_t size)
{
    // 边界检查：若数组为空，差值为 0.0
    if (size == 0)
    {
        return 0.0;
    }

    // 均以第 0 个元素作为基准初始值
    double max_val = arr[0];
    double min_val = arr[0];

    // 从第 1 个元素开始单次遍历比较
    for (size_t i = 1; i < size; i++)
    {
        if (arr[i] > max_val)
        {
            max_val = arr[i];
        }
        else if (arr[i] < min_val) // 如果已经是最大值，则不可能同时是最小值，用 else if 优化
        {
            min_val = arr[i];
        }
    }

    return max_val - min_val;
}

int main(void)
{
    // 测试用例 1：常规正浮点数
    double data1[5] = { 1.5, 3.2, 9.8, 2.1, 4.4 };
    // 测试用例 2：包含正数、负数与零
    double data2[6] = { -12.5, 0.0, 5.5, -3.2, 18.0, 7.3 };
    // 测试用例 3：全为相同数值（差值应为 0）
    double data3[4] = { 3.14, 3.14, 3.14, 3.14 };

    printf("data1 差值 (9.8 - 1.5): %.2f\n", get_array_range(data1, 5));
    printf("data2 差值 (18.0 - (-12.5)): %.2f\n", get_array_range(data2, 6));
    printf("data3 差值 (全相同): %.2f\n", get_array_range(data3, 4));

    return 0;
}
