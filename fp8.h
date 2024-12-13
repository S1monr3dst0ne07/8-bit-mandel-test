#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define SIGN 0x80
#define EXP_BIAS (3)

typedef uint8_t fp8;

/*
 * fp8.4.3: seeeemmm
 * fp8.3.4: seeemmmm
 *
*/

#define sgnSize_c 1
#define expSize_c 4
#define manSize_c 3

#define manVirtSize_c (manSize_c + 1)

#define sgnOffset_c (expOffset_c + expSize_c)
#define expOffset_c (manOffset_c + manSize_c)
#define manOffset_c 0

#define sgnMask_c (0xff >> (8 - sgnSize_c)) 
#define expMask_c (0xff >> (8 - expSize_c))
#define manMask_c (0xff >> (8 - manSize_c)) 

#define manVirtBit_c (manMask_c + 1)
#define manVirtMask_c (manVirtBit_c | manMask_c)
#define expSignExt_c (~expMask_c)
#define expSignBit_c (1 << (expSize_c - 1))

#define expMin_c (uint8_t)(0xff << (expSize_c - 1))
#define expMax_c (uint8_t)(~expMin_c)

//the most significant outta 8
#define int8Msb_c 0xf0

uint8_t getMan(fp8 x)
{
    return (x & manMask_c) | manVirtBit_c; 
}

int8_t getExp(fp8 x)
{
    int8_t exp = (x >> expOffset_c) & expMask_c;
    //make sure signs of (byte)exp and exp match
    if (exp & expSignBit_c) exp |= expSignExt_c; 
    return exp - EXP_BIAS;
}

fp8 render(uint8_t man, uint8_t exp, bool sgn)
{
    return 
        ((man & manMask_c) << manOffset_c) | 
        ((exp & expMask_c) << expOffset_c) | 
        ((sgn & sgnMask_c) << sgnOffset_c) ; 
}

fp8 normal(uint8_t man, uint8_t exp, bool sgn)
{
    //zero -> most negative exponent
    if (!man)
        return render(0, expMin_c, sgn);
    
    //compensate overflow
    while (man > manVirtMask_c)
    {
        man >>= 1;
        exp++;
    }
    //align man to virual bit
    while (!(man & manVirtBit_c))
    {
        man <<= 1;
        exp--; 
    }

    exp += EXP_BIAS;

    //clamp exponent
 
    if ( //underflow
      !(exp & expSignBit_c) && //no overflow
       (exp & int8Msb_c   )    //but negative
    ) exp = expMin_c;

    if ( //overflow
       (exp & expSignBit_c) && //overflow
      !(exp & int8Msb_c   )    //not negative
    ) exp = expMax_c;

    return render(man, exp, sgn);
}

fp8 value2fp(int x)
{
    uint8_t exp = 0;
    while (x >> manVirtSize_c)
    {
        x >>= 1;
        exp++;
    }

    return normal(x, exp, false);
}

float fp2float(fp8 x)
{
    return getMan(x) * pow(2.0f, getExp(x)) * ((x & SIGN) ? -1 : 1);
}
void print(fp8 x)
{
    printf("%f\n", fp2float(x));
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
	if (valLow & SIGN) invByte(&valLowMantissaMod);
	if (valBig & SIGN) invByte(&valBigMantissaMod);

	//do the addition
	int8_t outputMantissa = valBigMantissaMod + valLowMantissaMod;
	int8_t outputSign = 0;

	//check for sign and get abs
	if (outputMantissa & SIGN)
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
   
	int8_t sign = (val1 & SIGN) ^ (val2 & SIGN);


	return normal(outputMantissa, outputExponent, 0) | (sign ? SIGN : 0);
}


//does abs(val1 / val2)
fp8 div(fp8 val1, fp8 val2)
{
  //val1 mantissa is shifted to the right as much as possible to compensate for precision loss
  //val1 exponent is decremented to keep val1 the same
  const int precCompensateOffset = (8 - manVirtSize_c);
	uint8_t val1Mantissa = ((int8_t)getMan(val1)) << precCompensateOffset;
	uint8_t val2Mantissa = (int8_t)getMan(val2);

	int8_t val1Exponent = getExp(val1) - precCompensateOffset;
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

