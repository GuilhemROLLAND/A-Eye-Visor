#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<math.h>
#include<time.h>
#include<pthread.h>
#include <byteswap.h>

unsigned char img[921600];
unsigned char payload[50];
// unsigned char *ptr_img =0x1ffeae00;

typedef struct __attribute__((__packed__)) __attribute__ ((aligned)) BITMAPFILEHEADER
{
    unsigned short fType;  //specifies the file type
    unsigned int fSize;  //specifies the size in bytes of the bitmap file
    unsigned short fReserved1;  //reserved; must be 0
    unsigned short fReserved2;  //reserved; must be 0
    unsigned int fOffBits;  //specifies the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;

typedef struct __attribute__((__packed__)) BITMAPINFOHEADER
{
    unsigned int size;  //specifies the number of bytes required by the struct
    unsigned int width;  //specifies width in pixels
    unsigned int height;  //specifies height in pixels
    unsigned short planes;  //specifies the number of color planes, must be 1
    unsigned short bitCount;  //specifies the number of bits per pixel
    unsigned int compression;  //specifies the type of compression
    unsigned int sizeImage;  //size of image in bytes
    unsigned int xPelsPerMeter;  //number of pixels per meter in x axis
    unsigned int yPelsPerMeter;  //number of pixels per meter in y axis
    unsigned int clrUsed;  //number of colors used by the bitmap
    unsigned int clrImportant;  //number of colors that are important
}BITMAPINFOHEADER;
/**
 * @brief 
 * Take an address of a starting image in DDR (theorically) and extract informations from BMP 
 * header to corresponding structure. Place the image data into an array of corresponding size
 * @param addr starting address of a BMP file
 * @return bmpImg array containing the image data as [px1R,px1G,px1B,px2R,px2G,px2B] 
 */
unsigned char* readImgBmp(int addr) {
    unsigned char *bmpImg;
    unsigned char tempRGB;
    unsigned char *ptr_img = 0x7fffffed1960;
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
/**
 * @brief 
 * This function load a BMP file of a fixed size (640*480) into an array
 * @param testFilename filename
 * @return unsigned char* pointer to the array
 */
unsigned char* readImgBmpTest(char *testFilename) {
    FILE *filePtr;  //our file pointer
    BITMAPFILEHEADER bitmapFileHeader;  //our bitmap file header
    BITMAPINFOHEADER *bitmapInfoHeader;  //our bitmap info header
    unsigned char *bitmapImage;  //store image data
    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable

    //open file in read binary mode
    filePtr = fopen(testFilename,"rb");
    if (filePtr == NULL)
        return NULL;

    unsigned char fileData[1228938];
    fread(fileData, 1228938, 1, filePtr);
    unsigned char* addr = &fileData[0];
    return addr;
}


unsigned char* resizeImg(unsigned char* bmpImg) 
{
    unsigned char* bmpImgResized;
    bmpImgResized = (unsigned char*) malloc(691200*sizeof(unsigned char));
    for (int n = 0; n < 480; n++)
    {
        for (int i = (80*3+(n*640)*3); i<=((640*3*(n+1))-80*3); i++)
        {
            bmpImgResized[i-(240+(480*(n)))] = bmpImg[i];
            //printf("index du tableau : %d, shoud be 691200 in the end\n", i-(240+(480*(n))));
        }
    }
    return bmpImgResized;
}

unsigned char* avgPooling(unsigned char* img)
{
    int lengthImgData = 48, poolingLength = 4;
    int countR = 0, countG = 1, countB = 2, idx=0;
    int avgR = 0, avgG = 0, avgB = 0;
    int width = 12, length = 4;
    int pxR[(length*width)/3], pxG[(length*width)/3], pxB[(length*width)/3];
    unsigned char count = 0;
    unsigned char* imgPooled;
    imgPooled = (unsigned char*) malloc(((width*length)/2)*sizeof(unsigned char));
    for (int i = 0; i < (width*length)/(3*poolingLength); i)
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
    for (int i = 0; i < (width*length)/(3*poolingLength); i++) 
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
void testavgPooling() 
{
    unsigned char tabTest[48] = 
    {
    0,0,254,
    0,0,0,
    0,0,0,
    254,0,0,
    0,0,254,
    0,0,0,
    0,0,0,
    254,0,0,
    0,0,254,
    0,0,0,
    0,0,0,
    254,0,0,
    0,0,254,
    0,0,0,
    0,0,0,
    254,0,0
    };
    unsigned char *ret = avgPooling(tabTest);
    for (int i = 0; i<12; i++)
    {
        printf("%d,", ret[i]);
    }
}

int main() 
{
    char *testFilename = "pict.bmp";
    unsigned char* addr = readImgBmpTest(testFilename); 
    printf("addr: %p \n", addr);
    unsigned char *bitmapData = readImgBmp(addr);
    printf("addresse de depart du tableau de valeurs d'image : %p\n", bitmapData);
    printf("Image data byte 1 : %x, should be 1a\n", bitmapData[0]);
    printf("Image data byte 2 : %x, should be 2C\n", bitmapData[1]);
    printf("Image data byte 3 : %x, should be 74\n", bitmapData[2]);
    printf("Image data byte 4 : %x, should be 1a\n", bitmapData[3]);
    // unsigned char *imgResized = resizeImg(bitmapData);
    // printf("test : %x\n", imgResized[0]);
    testavgPooling();
    return 0; 
}