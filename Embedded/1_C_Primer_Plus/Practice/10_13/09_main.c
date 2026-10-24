#include <stdio.h>

#define ROWS 3
#define COLS 5

// 使用变长数组形参拷贝任意 rows×cols 的二维数组
void copy_2d(int rows, int cols, double target[rows][cols], double source[rows][cols])
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            target[i][j] = source[i][j];
        }
    }
}

// 使用变长数组形参显示任意 rows×cols 的二维数组
void show_2d(int rows, int cols, double arr[rows][cols])
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%6.1f", arr[i][j]);
        }
        printf("\n");
    }
}

int main(void)
{
    double source[ROWS][COLS] = {
        { 1.1, 2.2, 3.3, 4.4, 5.5 },
        { 6.6, 7.7, 8.8, 9.9, 10.1 },
        { 11.2, 12.3, 13.4, 14.5, 15.6 }
    };
    double target[ROWS][COLS];

    copy_2d(ROWS, COLS, target, source);

    printf("源数组 source:\n");
    show_2d(ROWS, COLS, source);
    printf("目标数组 target:\n");
    show_2d(ROWS, COLS, target);

    return 0;
}
