#include <stdio.h>

int main(void)
{
    float num = 0.0f;

    printf("Enter a floating-point value: ");
    if (scanf("%f", &num) != 1) {
        printf("Invalid input.\n");
        return 1;
    }

    printf("fixed-point notation: %f\n", num);
    printf("exponential notation: %e\n", num);
    printf("p notation: %a\n", num);

    return 0;
}
