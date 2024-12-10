#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>


#define SIGN_MASK 0x80
typedef uint8_t fp8;


uint8_t getMan(fp8 x)
{
    return (x & 0b111) | 0b1000; 
}

int8_t getExp(fp8 x)
{
    int8_t exp = (x >> 3) & 0b1111;
    return exp & 0b1000 ? exp | 0b11110000 : exp;
}

fp8 render(uint8_t man, uint8_t exp, bool sign)
{
    return (man & 0b111) | ((exp & 0b1111) << 3) | (sign << 7);
}

fp8 normal(uint8_t man, uint8_t exp, bool sign)
{
    if (man == 0)
        return render(0, 0x8, sign);
    
    while (!(man >> 3))
    {
        man <<= 1;
        exp--; 
    }

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

	//check for carry overflow
	if (outputMantissa & (1 << 4))
	{
		outputMantissa >>= 1;
		outputExponent += 1;
	}



	return normal(outputMantissa, outputExponent, outputSign);

}



fp8 mul(fp8 val1, fp8 val2)
{
	int8_t val1Mantissa = (int8_t)getMan(val1);
	int8_t val2Mantissa = (int8_t)getMan(val2);

     int8_t outputExponent = getExp(val1) + getExp(val2);             
	uint8_t outputMantissa = val1Mantissa * val2Mantissa;
    
    if (val1 == value2fp(0)) return val1;
    if (val2 == value2fp(0)) return val2;
    
	int8_t sign = (val1 & SIGN_MASK) ^ (val2 & SIGN_MASK);

	//move the outputMantissa back till it fit back into the char, in the process discarding the precision the we can't keep
	while ((unsigned)outputMantissa > (1 << 4) - 1)
	{
		outputMantissa >>= 1;
		outputExponent += 1;
	}


	return render(outputMantissa, outputExponent, 0) | (sign ? SIGN_MASK : 0);
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

	return render(outputMantissa, outputExponent, 0); 
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

void print(fp8 x)
{
    float v = getMan(x) * pow(2.0f, getExp(x)) * (x & SIGN_MASK ? -1 : 1);
    printf("%f\n", v);
}



int main()
{
    fp8 zero = value2fp(0);

    
    const int iXmax = 100;
    const int iYmax = 100;
        
    /*
    const double CxMin=-2;
    const double CxMax= 1;
    const double CyMin=-2.0;
    const double CyMax= 2.0;
    */
    
    fp8 CxMin = value2fp(2) | SIGN_MASK;
    fp8 CxMax = value2fp(1)            ;
    fp8 CyMin = value2fp(2) | SIGN_MASK;
    fp8 CyMax = value2fp(2)            ;
    
    /*
    double PixelWidth  = (CxMax-CxMin) / iXmax;
    double PixelHeight = (CyMax-CyMin) / iYmax;
    */
    
    fp8 PixelWidth  = div(add(CxMax, CxMin ^ SIGN_MASK), value2fp(iXmax));
    fp8 PixelHeight = div(add(CyMax, CyMin ^ SIGN_MASK), value2fp(iYmax));

    const int maxIter=10;

    FILE* fp = fopen("new.ppm","wb");
    fprintf(fp,"P6\n %d\n %d\n 255\n", iXmax, iYmax);

    for(int iY=0; iY<iYmax; iY++)
    for(int iX=0; iX<iXmax; iX++)
    {   
        /*
        double Cx=CxMin + iX*PixelWidth;
        double Cy=CyMin + iY*PixelHeight;
        */
        fp8 marX = mul(PixelWidth,  value2fp(iX));
        fp8 marY = mul(PixelHeight, value2fp(iY));

        fp8 Cx = add(CxMin, marX);
        fp8 Cy = add(CyMin, marY);

        /*
        double Zx=0.0;
        double Zy=0.0;
        double Zx2=Zx*Zx;
        double Zy2=Zy*Zy;
        */
        
        fp8 Zx  = zero;
        fp8 Zy  = zero;
        fp8 Zx2 = zero;
        fp8 Zy2 = zero;
        

        int iter = 0;
        //for (; iter < maxIter && ((Zx2+Zy2) < 4.0); iter++)
        for (; iter < maxIter && comp(value2fp(4), add(Zx2, Zy2)); iter++)
        {
            //printf("%d %d\n", Zy, Zx);
            Zy = add(mul(mul(Zx, Zy), value2fp(2)), Cy);
            Zx = add(add(Zx2, Zy2 ^ SIGN_MASK), Cx);
            Zx2 = mul(Zx, Zx);
            Zy2 = mul(Zy, Zy);

            
        };
                
        double gray = ((double)iter / maxIter) * 255.0;

        static unsigned char colors[3];
        colors[0]=(int)gray;
        colors[1]=(int)gray;
        colors[2]=(int)gray;

        fwrite(colors, 1, 3, fp);
    }

    fclose(fp);
    return 0;
}
