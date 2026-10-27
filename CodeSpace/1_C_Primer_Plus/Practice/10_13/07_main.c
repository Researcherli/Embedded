#include <stdio.h>

#define ROWS 3
#define COLS 4

// 来自练习 2 的一维数组拷贝函数（这里采用通用的下标法）
void copy_arr(double target[], const double source[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        target[i] = source[i];
    }
}

// 辅助打印二维数组的函数
void print_2d_arr(const double arr[][COLS], size_t rows)
{
    for (size_t i = 0; i < rows; i++)
    {
        printf("Row %zu: [ ", i);
        for (size_t j = 0; j < COLS; j++)
        {
            printf("%5.1f", arr[i][j]);
        }
        printf(" ]\n");
    }
}

int main(void)
{
    // 初始化 double 类型的二维数组 (3行4列)
    double source[ROWS][COLS] = {
        { 1.1, 1.2, 1.3, 1.4 },
        { 2.1, 2.2, 2.3, 2.4 },
        { 3.1, 3.2, 3.3, 3.4 }
    };

    double target[ROWS][COLS];

    // 利用一维拷贝函数，逐行完成二维数组的拷贝
    for (size_t i = 0; i < ROWS; i++)
    {
        // source[i] 表示第 i 行子数组的首地址，COLS 为该行的元素个数
        copy_arr(target[i], source[i], COLS);
    }

    // 打印对比验证
    printf("--- 源数组 source ---\n");
    print_2d_arr(source, ROWS);

    printf("\n--- 目标数组 target ---\n");
    print_2d_arr(target, ROWS);

    return 0;
}
