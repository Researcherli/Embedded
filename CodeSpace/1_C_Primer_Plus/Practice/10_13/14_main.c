#include <stdio.h>

#define ROWS 3
#define COLS 5

// a. 把用户输入的数据储存在 rows×cols 的数组中
void input_arr(int rows, int cols, double arr[rows][cols])
{
    for (int i = 0; i < rows; i++)
    {
        printf("请输入第 %d 组的 %d 个数: ", i + 1, cols);
        for (int j = 0; j < cols; j++)
        {
            scanf("%lf", &arr[i][j]);
        }
    }
}

// b. 计算一维数组的平均值
double row_average(int size, const double arr[size])
{
    double sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += arr[i];
    }
    return sum / size;
}

// c. 计算所有数据的平均值
double all_average(int rows, int cols, double arr[rows][cols])
{
    double sum = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            sum += arr[i][j];
        }
    }
    return sum / (rows * cols);
}

// d. 找出所有数据中的最大值
double max_value(int rows, int cols, double arr[rows][cols])
{
    double max = arr[0][0];
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
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
void print_result(int rows, int cols, double arr[rows][cols])
{
    printf("\n输入的数据:\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%8.2f", arr[i][j]);
        }
        printf("\n");
    }
}

int main(void)
{
    double arr[ROWS][COLS];

    input_arr(ROWS, COLS, arr);
    print_result(ROWS, COLS, arr);

    for (int i = 0; i < ROWS; i++)
    {
        printf("第 %d 组的平均值: %.2f\n", i + 1, row_average(COLS, arr[i]));
    }
    printf("所有数据的平均值: %.2f\n", all_average(ROWS, COLS, arr));
    printf("最大值: %.2f\n", max_value(ROWS, COLS, arr));

    return 0;
}
