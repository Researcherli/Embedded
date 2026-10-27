#include <stdio.h>
#include <limits.h>
#include <float.h>

int main(void) {
    printf("================ 1. 整数上溢试验 ================\n");
    // 有符号整型最大值：INT_MAX (通常为 2147483647)
    int max_signed_int = INT_MAX;
    printf("加 1 前 (INT_MAX): %d\n", max_signed_int);

    // 强制防优化操作观察底层补码回绕
    volatile int overflow_int = max_signed_int + 1;
    printf("加 1 后 (未定义行为/硬件回绕): %d (等于 INT_MIN: %d)\n\n",
           overflow_int, INT_MIN);

    printf("================ 2. 浮点数上溢试验 ================\n");
    // 单精度浮点数最大值：FLT_MAX (约 3.402823e+38)
    float max_float = FLT_MAX;
    printf("乘 2 前 (FLT_MAX): %e\n", max_float);

    volatile float overflow_float = max_float * 2.0f;
    printf("乘 2 后 (上溢至无穷大): %f (科学计数法: %e)\n\n",
           overflow_float, overflow_float);

    printf("================ 3. 浮点数下溢试验 ================\n");
    // 单精度最小正规格化数：FLT_MIN (约 1.175494e-38)
    float min_float = FLT_MIN;
    printf("除 2 前 (FLT_MIN): %e\n", min_float);

    // 渐进下溢：进入非规格化数阶段
    volatile float subnormal_float = min_float / 2.0f;
    printf("除以 2 后 (非规格化数): %e\n", subnormal_float);

    // 严重下溢：完全超出可表示范围
    volatile float underflow_zero = min_float / 1.0e10f;
    printf("除以 10^10 后 (完全下溢至零): %e\n", underflow_zero);

    return 0;
}
