
#include "fp8.h"


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
    
    fp8 CxMin = value2fp(2) | SIGN;
    fp8 CxMax = value2fp(1)       ;
    fp8 CyMin = value2fp(2) | SIGN;
    fp8 CyMax = value2fp(2)       ;
    
    /*
    double PixelWidth  = (CxMax-CxMin) / iXmax;
    double PixelHeight = (CyMax-CyMin) / iYmax;
    */
    
    fp8 PixelWidth  = div(add(CxMax, CxMin ^ SIGN), value2fp(iXmax));
    fp8 PixelHeight = div(add(CyMax, CyMin ^ SIGN), value2fp(iYmax));

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
            Zx = add(add(Zx2, Zy2 ^ SIGN), Cx);
            Zx2 = mul(Zx, Zx);
            Zy2 = mul(Zy, Zy);
            
        };
                
        double gray = ((double)iter / maxIter) * 255.0;

        static unsigned char colors[3];
        colors[0]=(int)gray;
        colors[1]=(int)0;
        colors[2]=(int)gray;

        fwrite(colors, 1, 3, fp);
    }

    fclose(fp);

    printf("done\n");
    return 0;
}
