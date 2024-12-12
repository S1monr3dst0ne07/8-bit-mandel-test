#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define SIGN_MASK 0x80
#define EXP_BIAS (3)

typedef uint8_t fp8;


uint8_t getMan(fp8 x)
{
    return (x & 0b111) | 0b1000; 
}

int8_t getExp(fp8 x)
{
    int8_t exp = (x >> 3) & 0b1111;
    exp = exp | (exp & 0x8 ? 0xf0 : 0x00);
    return exp - EXP_BIAS;
}

fp8 render(uint8_t man, uint8_t exp, bool sign)
{
    return 
        (man & 0b111) | 
        ((exp & 0b1111) << 3) | 
        (sign << 7);
}

fp8 normal(uint8_t man, uint8_t exp, bool sign)
{
    if (man == 0)
        return render(0, 0x8, sign);
    
    while ((unsigned)man > 0xf)
    {
        man >>= 1;
        exp++;
    }
    while (!(man >> 3))
    {
        man <<= 1;
        exp--; 
    }

    exp += EXP_BIAS;

    if (!(exp & 0x08) && (exp & 0xf0))
        exp = 0xf8;
    if ((exp & 0x08) && !(exp & 0xf0))
        exp = 0x07;

    return render(man, exp, sign);
}

fp8 value2fp(int x)
{
    uint8_t exp = 0;
    while (x >> 4)
    {
        x >>= 1;
        exp++;
    }

    return normal(x, exp, false);
}



void invByte(int8_t* value)
{
	*value = ~(*value) + 1;
}


fp8 add(fp8 val1, fp8 val2)
{
	fp8 valLow = 0;
	fp8 valBig = 0;

	if (getExp(val1) < getExp(val2))
	{
		valBig = val2;
		valLow = val1;
	}
	else
	{
		valBig = val1;
		valLow = val2;
	}

	char outputExponent = getExp(valBig);
	char deltaExponent  = outputExponent - getExp(valLow);

	//cast to short for the addition to catch carry
	int8_t valBigMantissaMod = (int8_t)getMan(valBig);
	int8_t valLowMantissaMod = (int8_t)getMan(valLow) >> deltaExponent;

	//apply the sign
	if (valLow & SIGN_MASK) invByte(&valLowMantissaMod);
	if (valBig & SIGN_MASK) invByte(&valBigMantissaMod);

	//do the addition
	int8_t outputMantissa = valBigMantissaMod + valLowMantissaMod;
	int8_t outputSign = 0;

	//check for sign and get abs
	if (outputMantissa & SIGN_MASK)
	{
		outputSign = 1;
		invByte(&outputMantissa);

	}


	return normal(outputMantissa, outputExponent, outputSign);

}



fp8 mul(fp8 val1, fp8 val2)
{
	int8_t val1Mantissa = (int8_t)getMan(val1);
	int8_t val2Mantissa = (int8_t)getMan(val2);

     int8_t outputExponent = getExp(val1) + getExp(val2);             
	uint8_t outputMantissa = val1Mantissa * val2Mantissa;
   
	int8_t sign = (val1 & SIGN_MASK) ^ (val2 & SIGN_MASK);


	return normal(outputMantissa, outputExponent, 0) | (sign ? SIGN_MASK : 0);
}


//does val1 / val2
fp8 div(fp8 val1, fp8 val2)
{
	uint8_t val1Mantissa = ((int8_t)getMan(val1)) << 4;
	uint8_t val2Mantissa = (int8_t)getMan(val2);

	int8_t val1Exponent = getExp(val1) - 4;
	int8_t val2Exponent = getExp(val2);

	int8_t outputExponent = val1Exponent - val2Exponent;
	uint8_t outputMantissa = val1Mantissa / val2Mantissa;

	return normal(outputMantissa, outputExponent, 0); 
}


//return 1 if val1 > val2
int comp(fp8 big, fp8 small)
{
	int8_t val1Exponent = getExp(big);
	int8_t val2Exponent = getExp(small);

	int8_t val1Mantissa = getMan(big);
	int8_t val2Mantissa = getMan(small);

	if (val1Exponent != val2Exponent)
		return val1Exponent > val2Exponent;
	else
		return val1Mantissa > val2Mantissa;
}

float fp2float(fp8 x)
{
    return getMan(x) * pow(2.0f, getExp(x)) * ((x & SIGN_MASK) ? -1 : 1);
}
void print(fp8 x)
{
    printf("%f\n", fp2float(x));
}


