#include <stdio.h>

int main(void)
{
    int val = 0;

    printf("Please enter one ASCII code: ");
    if (scanf("%d", &val) == 1) {
        printf("This val char is: %c\n", val);
    } else {
        printf("Invalid input.\n");
    }
    
    return 0;
}
