#include <stdio.h>

int main(void)
{
    int height = 0;
    scanf("%d", &height);
    printf("%.2e cm\n", height * 2.54);
    return 0;
}
