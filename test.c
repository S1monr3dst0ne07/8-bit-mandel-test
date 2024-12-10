
#include "fp8.h"

void testAdd()
{
    for (int a = 0; a < 256; a++)
    for (int b = 0; b < 256; b++)
    {
        float real = fp2float(a) + fp2float(b);
        float virt = fp2float(add(a, b));

        if (real != virt)
        {
            printf("a: %d, b: %d\n", a, b);
            printf("real: %f, virt: %f\n", real, virt);
        }
    }
}

void testMul()
{
    for (int a = 0; a < 256; a++)
    for (int b = 0; b < 256; b++)
    {
        float real = fp2float(a) * fp2float(b);
        float virt = fp2float(mul(a, b));

        if (real != virt)
        {
            printf("a: %d, b: %d\n", a, b);
            printf("real: %f, virt: %f\n", real, virt);
        }
    }
}

void testConv()
{
    for (int a = 0; a < 255; a++)
    {
        printf("int: %d, real: %f\n", a, fp2float(a));
    }
}

int main()
{
    fp8 acc = value2fp(0);
    fp8 run = value2fp(1);
    fp8 half = div(value2fp(1), value2fp(2));

    for (int i = 0; i < 10; i++)
    {
        print(acc);
        acc = add(acc, run);
        run = mul(run, half);
    }

}
