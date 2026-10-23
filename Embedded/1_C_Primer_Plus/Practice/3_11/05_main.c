#include <stdio.h>

void Seconds(int age)
{
    int seconds = 0;
    seconds = age * 3.156e7;
    printf("%d\n", seconds);
}

int main(void)
{
    int age = 0;
    scanf("%d", &age);
    Seconds(age);
    return 0;
}
