#include <stdio.h>
#include <string.h>

void copy_arr(double target1[], const double source[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        target1[i] = source[i];
    }
}

void copy_ptr(double *target2, const double *source, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        *target2++ = *source++;
    }
}

void copy_ptrs(double *target3, const double *source_start, double *source_end)
{
    while (source_start < source_end)
    {
        *target3++ = *source_start++;
    }
}

int main(void)
{
    double source[5] = { 1.1, 2.2, 3.3, 4.4, 5.5};
    double target1[5];
    double target2[5];
    double target3[5];

    copy_arr(target1, source, 5);
    copy_ptr(target2, source, 5);
    copy_ptrs(target3, source, source + 5);

    for(int i = 0; i < 5; i++)
    {
        printf("target1[%d] = %f\n", i, target1[i]);
    }
    for (int i = 0; i < 5; i++)
    {
        printf("target2[%d] = %f\n", i, target2[i]);
    }
    for (int i = 0; i < 5; i++)
    {
        printf("target3[%d] = %f\n", i, target3[i]);
    }
    return 0;
}
