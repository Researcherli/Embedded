#include <stdio.h>

int main(void)
{
    float cups;

    printf("请输入杯数: ");
    scanf("%f", &cups);

    float pints   = cups / 2.0f;          // 1品脱 = 2杯
    float ounces  = cups * 8.0f;          // 1杯 = 8盎司
    float spoons  = ounces * 2.0f;        // 1盎司 = 2大汤勺
    float teaspoons = spoons * 3.0f;      // 1大汤勺 = 3茶勺

    printf("%.2f 杯等于:\n", cups);
    printf("  %.2f 品脱\n", pints);
    printf("  %.2f 盎司\n", ounces);
    printf("  %.2f 大汤勺\n", spoons);
    printf("  %.2f 茶勺\n", teaspoons);

    return 0;
}
