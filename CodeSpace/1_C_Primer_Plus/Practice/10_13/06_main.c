#include <stdio.h>

// 原地反转 double 数组：将首尾两端的元素对调向中间靠拢
void reverse_arr(double arr[], size_t size)
{
    if (size <= 1)
    {
        return; // 0 个或 1 个元素无需倒序
    }

    size_t left = 0;
    size_t right = size - 1;

    while (left < right)
    {
        // 交换两端的元素
        double temp = arr[left];
        arr[left] = arr[right];
        arr[right] = temp;

        // 两端向中间推进
        left++;
        right--;
    }
}

// 辅助打印函数
void print_arr(const double arr[], size_t size)
{
    printf("{ ");
    for (size_t i = 0; i < size; i++)
    {
        printf("%.2f", arr[i]);
        if (i + 1 < size)
        {
            printf(", ");
        }
    }
    printf(" }\n");
}

int main(void)
{
    // 测试用例 1：奇数长度数组
    double odd_arr[5] = { 1.1, 2.2, 3.3, 4.4, 5.5 };
    // 测试用例 2：偶数长度数组
    double even_arr[6] = { 10.0, 20.0, 30.0, 40.0, 50.0, 60.0 };

    printf("--- 测试 1：奇数长度 (5 个元素) ---\n");
    printf("反转前: ");
    print_arr(odd_arr, 5);
    reverse_arr(odd_arr, 5);
    printf("反转后: ");
    print_arr(odd_arr, 5);

    printf("\n--- 测试 2：偶数长度 (6 个元素) ---\n");
    printf("反转前: ");
    print_arr(even_arr, 6);
    reverse_arr(even_arr, 6);
    printf("反转后: ");
    print_arr(even_arr, 6);

    return 0;
}
