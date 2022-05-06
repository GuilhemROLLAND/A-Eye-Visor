#include "preprocess.h"

// unsigned char *ptr_img =0x1ffeae00;

unsigned char* readImg(unsigned char* addr, int width, int height) {
    unsigned char *img;
    unsigned char *ptr_img = (unsigned char*) addr;
    if ((img = malloc((width*height*3) * sizeof(unsigned char))) == NULL)
        printf("erreur allocation mémoire \n");
    img[0] = ptr_img;
    return img;   
}

unsigned char* resizeImg(unsigned char* bmpImg, int width, int height, int pixelsToCrop) 
{
    unsigned char* bmpImgResized;
    if ((bmpImgResized = malloc((width*height*3 - pixelsToCrop*3)*sizeof(unsigned char))) == NULL)
        printf("erreur allocation mémoire \n");
    for (int n = 0; n < height; n++)
    {
        for (int i = ((pixelsToCrop/2)*3+(n*width)*3); i<((width*3*(n+1))-(pixelsToCrop/2)*3); i++)
        {
            bmpImgResized[i-((pixelsToCrop/2)*3+(height*(n)))] = bmpImg[i];
        }
    }
    return bmpImgResized;
}



unsigned char* avgPooling(unsigned char* img, int width, int height, unsigned char poolingLength)
{
    unsigned char lengthImgData = width * height;
    unsigned char countR = 0, countG = 1, countB = 2, idx=0;
    unsigned int avgR = 0, avgG = 0, avgB = 0;
    unsigned char pxR[(height*width)/3], pxG[(height*width)/3], pxB[(height*width)/3];
    unsigned char count = 0;
    unsigned char* imgPooled;
    if ((imgPooled = (unsigned char*) malloc(((width*height)/2)*sizeof(unsigned char))) == NULL)
        printf("erreur allocation mémoire \n");
    for (int i = 0; i < (width*height)/(3*poolingLength); i)
    {   
        for (int n=0; n < poolingLength; n++)
        {
            if (n == 1) 
            {
                pxR[idx] = img[countR];
                pxG[idx] = img[countG];
                pxB[idx] = img[countB];
                countR += (width-3);
                countG += (width-3);
                countB += (width-3);
                idx++;
                continue;
            }
            pxR[idx] = img[countR];
            pxG[idx] = img[countG];
            pxB[idx] = img[countB];
            countR += 3;
            countG += 3;
            countB += 3;
            idx++;
        }
        i++;
        if (i%2 != 0) 
        {
            countR = width*((i)-0.5);
            countG = width*((i)-0.5) + 1;
            countB = width*((i)-0.5) + 2;
        }         
    }
    for (int i = 0; i < (width*height)/(3*poolingLength); i++) 
    {
        avgR = 0;
        avgR = 0;
        avgB = 0;
        for (int n=0; n < poolingLength; n++)
        {
            avgR += pxR[count];
            avgG += pxG[count];
            avgB += pxB[count];
            count++;
        }
        avgR = avgR/4;
        pxR[i] = avgR;
        avgG = avgG/4;
        pxG[i] = avgG;
        avgB = avgB/4;
        pxB[i] = avgB;
    }
    countR = 0;
    countG = 1;
    countB = 2;
    for (int i=0; i< poolingLength; i++) 
    {
        imgPooled[countR] = pxR[i];
        imgPooled[countG] = pxG[i];
        imgPooled[countB] = pxB[i];
        countR += 3;
        countG += 3;
        countB += 3;
    }
    return imgPooled;
}


float* rescaling(unsigned char* img, int height, int width)  
{
    float* imgRescaled;
    if (( imgRescaled = malloc(height*width*sizeof(float))) == NULL)
        printf("erreur allocation mémoire \n");
    for (int i = 0; i < height*width*3; i++)
    {
        imgRescaled[i] =(float) ((img[i]/127.5) - 1);
    }
    return imgRescaled;
}

unsigned char* maxPooling(unsigned char* img, int width, int height, unsigned char poolingLength)
{
    unsigned char lengthImgData = width * height;
    unsigned char countR = 0, countG = 1, countB = 2, idx=0;
    unsigned int maxR = 0, maxG = 0, maxB = 0;
    unsigned char pxR[(height*width)/3], pxG[(height*width)/3], pxB[(height*width)/3];
    unsigned char count = 0;
    unsigned char* imgPooled;
    if ((imgPooled = (unsigned char*) malloc(((width*height)/2)*sizeof(unsigned char))) == NULL)
        printf("erreur allocation mémoire \n");
    for (int i = 0; i < (width*height)/(3*poolingLength); i)
    {   
        for (int n=0; n < poolingLength; n++)
        {
            if (n == 1) 
            {
                pxR[idx] = img[countR];
                pxG[idx] = img[countG];
                pxB[idx] = img[countB];
                countR += (width-3);
                countG += (width-3);
                countB += (width-3);
                idx++;
                continue;
            }
            pxR[idx] = img[countR];
            pxG[idx] = img[countG];
            pxB[idx] = img[countB];
            countR += 3;
            countG += 3;
            countB += 3;
            idx++;
        }
        i++;
        if (i%2 != 0) 
        {
            countR = width*((i)-0.5);
            countG = width*((i)-0.5) + 1;
            countB = width*((i)-0.5) + 2;
        }         
    }
    for (int i = 0; i < (width*height)/(3*poolingLength); i++) 
    {
        for (int n=0; n < poolingLength; n++)
        {
            
            maxG += pxG[count];
            maxB += pxB[count];
            if (maxR > pxR[count])
                maxR = pxR[count];
            if (maxG > pxG[count])
                maxG = pxG[count];
            if (maxB > pxB[count])
                maxB = pxB[count];
            count++;
        }
        pxR[i] = maxR;
        pxG[i] = maxG;
        pxB[i] = maxB;
    }
    countR = 0;
    countG = 1;
    countB = 2;
    for (int i=0; i< poolingLength; i++) 
    {
        imgPooled[countR] = pxR[i];
        imgPooled[countG] = pxG[i];
        imgPooled[countB] = pxB[i];
        countR += 3;
        countG += 3;
        countB += 3;
    }
    return imgPooled;
}
