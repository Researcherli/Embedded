#include <stdio.h>

#define SIZE 4

// 把 a、b 中相对应的元素相加，结果存入 sum
void add_arr(const int a[], const int b[], int sum[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        sum[i] = a[i] + b[i];
    }
}

void print_arr(const char *name, const int arr[], size_t size)
{
    printf("%s: [ ", name);
    for (size_t i = 0; i < size; i++)
    {
        printf("%d%s", arr[i], (i + 1 < size) ? ", " : "");
    }
    printf(" ]\n");
}

int main(void)
{
    int a[SIZE] = { 2, 4, 5, 8 };
    int b[SIZE] = { 1, 0, 4, 6 };
    int sum[SIZE];

    add_arr(a, b, sum, SIZE);

    print_arr("数组 1", a, SIZE);
    print_arr("数组 2", b, SIZE);
    print_arr("相加结果", sum, SIZE);

    return 0;
}
