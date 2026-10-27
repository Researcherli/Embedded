#include <stdio.h>

// 复用编程练习 2 中的一维数组拷贝函数（无需作任何修改）
void copy_arr(double target[], const double source[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        target[i] = source[i];
    }
}

// 辅助打印函数
void print_arr(const char *name, const double arr[], size_t size)
{
    printf("%s: [ ", name);
    for (size_t i = 0; i < size; i++)
    {
        printf("%.1f%s", arr[i], (i + 1 < size) ? ", " : "");
    }
    printf(" ]\n");
}

int main(void)
{
    // 源数组：包含 7 个元素
    // 对应序号：第1个(1.1), 第2个(2.2), 第3个(3.3), 第4个(4.4), 第5个(5.5), 第6个(6.6), 第7个(7.7)
    // 对应下标：  0          1          2          3          4          5          6
    double source[7] = { 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7 };

    // 目标数组：包含 3 个元素
    double target[3];

    // 核心调用：
    // 第 3 个元素的下标是 2，其地址可以写为 &source[2] 或 source + 2
    // 第 3 到 第 5 个元素共包含 3 个数据 (下标 2, 3, 4)
    copy_arr(target, source + 2, 3);
    // 也可以等价写为: copy_arr(target, &source[2], 3);

    // 打印对比
    print_arr("源数组 source (全部7个)", source, 7);
    print_arr("目标数组 target (第3~5个)", target, 3);

    return 0;
}
