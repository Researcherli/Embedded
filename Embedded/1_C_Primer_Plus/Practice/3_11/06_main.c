#include <stdio.h>

void WaterMolecules(int water)
{
    int quart = 950;
    double water_molecules;
    water_molecules = water * quart / 3.0e-23;
    printf("%.2e\n", water_molecules);
}

int main(void)
{
    int water = 0;
    scanf("%d", &water);
    WaterMolecules(water);
    return 0;
}
