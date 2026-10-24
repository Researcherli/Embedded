#include <stdio.h>

size_t find_max_index(const double *arr, size_t size)
{
    if (size == 0)
    {
        return 0;
    }

    size_t max_index = 0; // 假定第 0 个元素最大

    for (size_t i = 1; i < size; i++)
    {
        // 直接使用 double 类型的元素进行精度完整的比较
        if (arr[i] > arr[max_index])
        {
            max_index = i;
        }
    }

    return max_index;
}

int main(void)
{
    double source[5] = {1.1, 2.5, 6.8, 4.2, 5.0};
    double negative_test[4] = {-10.2, -3.1, -5.5, -8.0};

    size_t idx1 = find_max_index(source, 5);
    size_t idx2 = find_max_index(negative_test, 4);

    // 打印下标以及该下标对应的真实值
    printf("source: 最大值下标 = %zu, 对应的最大值 = %.1f\n", idx1, source[idx1]);
    printf("negative_test: 最大值下标 = %zu, 对应的最大值 = %.1f\n", idx2, negative_test[idx2]);

    return 0;
}
