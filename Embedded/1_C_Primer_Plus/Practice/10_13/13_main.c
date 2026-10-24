#include <stdio.h>

#define ROWS 3
#define COLS 5

// a. 把用户输入的数据储存在 3×5 的数组中
void input_arr(double arr[][COLS], int rows)
{
    for (int i = 0; i < rows; i++)
    {
        printf("请输入第 %d 组的 %d 个数: ", i + 1, COLS);
        for (int j = 0; j < COLS; j++)
        {
            scanf("%lf", &arr[i][j]);
        }
    }
}

// b. 计算一维数组的平均值
double row_average(const double arr[], int size)
{
    double sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += arr[i];
    }
    return sum / size;
}

// c. 计算所有数据的平均值
double all_average(double arr[][COLS], int rows)
{
    double sum = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            sum += arr[i][j];
        }
    }
    return sum / (rows * COLS);
}

// d. 找出所有数据中的最大值
double max_value(double arr[][COLS], int rows)
{
    double max = arr[0][0];
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (arr[i][j] > max)
            {
                max = arr[i][j];
            }
        }
    }
    return max;
}

// e. 打印结果
void print_result(double arr[][COLS], int rows)
{
    printf("\n输入的数据:\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            printf("%8.2f", arr[i][j]);
        }
        printf("\n");
    }
}

int main(void)
{
    double arr[ROWS][COLS];

    input_arr(arr, ROWS);
    print_result(arr, ROWS);

    for (int i = 0; i < ROWS; i++)
    {
        printf("第 %d 组的平均值: %.2f\n", i + 1, row_average(arr[i], COLS));
    }
    printf("所有数据的平均值: %.2f\n", all_average(arr, ROWS));
    printf("最大值: %.2f\n", max_value(arr, ROWS));

    return 0;
}
