#include "preprocess_ddr.h"

// unsigned char *ptr_img =0x1ffeae00;

unsigned char* readImgBmp(unsigned char* addr) {
    unsigned char *bmpImg;
    unsigned char tempRGB;
    unsigned char *ptr_img = (unsigned char*) addr;
    // initialisation du pointeur mémoire a l'adresse de démarrage
    BITMAPFILEHEADER* ptr_header = (BITMAPFILEHEADER*) ptr_img;
    BITMAPINFOHEADER* ptr_info = (BITMAPINFOHEADER*) (ptr_img +0xe);
    ptr_header->fType = __bswap_16(ptr_header->fType);
    ptr_header->fSize = __bswap_32(ptr_header->fSize);
    ptr_header->fReserved1 = __bswap_16(ptr_header->fReserved1);
    ptr_header->fReserved2 = __bswap_16(ptr_header->fReserved2);
    ptr_header->fOffBits = __bswap_32(ptr_header->fOffBits);
    ptr_info->size = __bswap_32(ptr_info->size);
    ptr_info->width = __bswap_32(ptr_info->width);
    ptr_info->height = __bswap_32(ptr_info->height);
    ptr_info->planes = __bswap_16(ptr_info->planes);
    ptr_info->bitCount = __bswap_16(ptr_info->bitCount);
    ptr_info->compression = __bswap_32(ptr_info->compression);
    ptr_info->sizeImage = __bswap_32(ptr_info->sizeImage);
    ptr_info->xPelsPerMeter = __bswap_32(ptr_info->xPelsPerMeter);
    ptr_info->yPelsPerMeter = __bswap_32(ptr_info->yPelsPerMeter);
    ptr_info->clrUsed = __bswap_32(ptr_info->clrUsed);
    ptr_info->clrImportant = __bswap_32(ptr_info->clrImportant);
    bmpImg  = (unsigned char*) malloc(ptr_info->sizeImage*sizeof(unsigned char));
    if (!bmpImg)
    {
        free(bmpImg);
        printf("probleme d'allocation\n");
        return NULL;
    }
    unsigned char* ptr_img_data = ptr_img +0x8a;
    for (int n = 0; n < __bswap_32(ptr_info->sizeImage); n++)
    {
        bmpImg[n] = *ptr_img_data;
        ptr_img_data += 1;
    }

    for (int i = 3; i < ptr_info->sizeImage; i+=4)
    {
        bmpImg[i] = bmpImg[i+1];
        bmpImg[i+1] = bmpImg[i+2];
        bmpImg[i+2] = bmpImg[i+3];
    }

    for (int i=0; i < ptr_info->sizeImage; i+=3)
    {
        tempRGB = bmpImg[i];
        bmpImg[i] = bmpImg[i + 2];
        bmpImg[i + 2] = tempRGB;
    }
    return bmpImg;
}

unsigned char* resizeImg(unsigned char* bmpImg, unsigned char width, unsigned char height, unsigned char pixelsToCrop) 
{
    unsigned char* bmpImgResized;
    bmpImgResized = (unsigned char*) malloc((width*height*3 - pixelsToCrop*3)*sizeof(unsigned char));
    for (int n = 0; n < height; n++)
    {
        for (int i = ((pixelsToCrop/2)*3+(n*width)*3); i<((width*3*(n+1))-(pixelsToCrop/2)*3); i++)
        {
            bmpImgResized[i-((pixelsToCrop/2)*3+(height*(n)))] = bmpImg[i];
        }
    }
    return bmpImgResized;
}



unsigned char* avgPooling(unsigned char* img,unsigned char width, unsigned char height, unsigned char poolingLength)
{
    unsigned char lengthImgData = width * height;
    unsigned char countR = 0, countG = 1, countB = 2, idx=0;
    unsigned int avgR = 0, avgG = 0, avgB = 0;
    unsigned char pxR[(height*width)/3], pxG[(height*width)/3], pxB[(height*width)/3];
    unsigned char count = 0;
    unsigned char* imgPooled;
    imgPooled = (unsigned char*) malloc(((width*height)/2)*sizeof(unsigned char));
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

/**
 * @brief Performs a rescaling on the pixels value. Change values from [0;255] to [0;1]
 * 
 * @param img input img stored as [pxR1,pxG1,pxB1,pxR2,pxG2,pxB2, ...]
 * @param height height of the image
 * @param width width of the image. must be *3 if the image is RGB 
 * @return float* 
 */
float* rescaling(unsigned char* img, unsigned char height, unsigned char width)  
{
    float* imgRescaled = (float*) malloc(height*width*sizeof(float));
    for (int i = 0; i < height*width*3; i++)
    {
        imgRescaled[i] =(float) img[i]/255;
    }
    return imgRescaled;
}


