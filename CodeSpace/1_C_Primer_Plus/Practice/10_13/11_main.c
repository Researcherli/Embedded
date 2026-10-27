#include <stdio.h>

#define ROWS 3
#define COLS 5

// 以数组名和行数作为参数显示数组内容
void show_arr(int arr[][COLS], int rows)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            printf("%4d", arr[i][j]);
        }
        printf("\n");
    }
}

// 以数组名和行数作为参数把各元素的值翻倍
void double_arr(int arr[][COLS], int rows)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            arr[i][j] *= 2;
        }
    }
}

int main(void)
{
    int arr[ROWS][COLS] = {
        { 1, 2, 3, 4, 5 },
        { 6, 7, 8, 9, 10 },
        { 11, 12, 13, 14, 15 }
    };

    printf("原数组:\n");
    show_arr(arr, ROWS);

    double_arr(arr, ROWS);

    printf("翻倍后的数组:\n");
    show_arr(arr, ROWS);

    return 0;
}
