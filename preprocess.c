#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<math.h>
#include<time.h>

typedef struct tagBITMAPFILEHEADER
{
    unsigned short bfType;  //specifies the file type
    unsigned long bfSize;  //specifies the size in bytes of the bitmap file
    unsigned short bfReserved1;  //reserved; must be 0
    unsigned short bfReserved2;  //reserved; must be 0
    unsigned long bfOffBits;  //specifies the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER
{
    unsigned long biSize;  //specifies the number of bytes required by the struct
    long biWidth;  //specifies width in pixels
    long biHeight;  //specifies height in pixels
    unsigned short biPlanes;  //specifies the number of color planes, must be 1
    unsigned short biBitCount;  //specifies the number of bits per pixel
    unsigned long biCompression;  //specifies the type of compression
    unsigned long biSizeImage;  //size of image in bytes
    long biXPelsPerMeter;  //number of pixels per meter in x axis
    long biYPelsPerMeter;  //number of pixels per meter in y axis
    unsigned long biClrUsed;  //number of colors used by the bitmap
    unsigned long biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;


unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr;  //our file pointer
    BITMAPFILEHEADER bitmapFileHeader;  //our bitmap file header
    unsigned char *bitmapImage;  //store image data
    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable

    //open file in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,filePtr);

    //verify that this is a .BMP file by checking bitmap id
    if (bitmapFileHeader.bfType !=0x4D42)
    {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr); 

    //move file pointer to the beginning of bitmap data
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

    //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    fread(bitmapImage,bitmapInfoHeader->biSizeImage,1,filePtr);

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //swap the R and B values to get RGB (bitmap is BGR)
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx+=3)
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    //close file and return bitmap image data
    fclose(filePtr);
    return bitmapImage;
}



int main() {
    BITMAPINFOHEADER bitmapInfoHeader;
    unsigned char *bitmapData;
    // ...
    bitmapData = LoadBitmapFile("pict.bmp",&bitmapInfoHeader);
    //now do what you want with it, later on I will show you how to display it in a normal window
    printf("size of image data : %d", bitmapInfoHeader.biWidth);
    // for (int i = 0; i<30; i++) {
    //     printf("%c\n\r",*(bitmapData));
    // } 
    return 0;
}
