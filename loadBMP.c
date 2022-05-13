#include <stdio.h>
#include "preprocess.h"

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
    if (bitmapFileHeader.fType !=0x4D42)
    {
        fclose(filePtr);
        printf("Mauvais format de fichier ! \n");
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr); 

    //move file pointer to the beginning of bitmap data
    fseek(filePtr, bitmapFileHeader.fOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->sizeImage);

    //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    fread(bitmapImage,bitmapInfoHeader->sizeImage,1,filePtr);

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //swap the R and B values to get RGB (bitmap is BGR)
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->sizeImage;imageIdx+=3)
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    //close file and return bitmap image data
    fclose(filePtr);
    return bitmapImage;
}

int main() 
{
    printf("allo ? \n");
    BITMAPINFOHEADER *bitmapinfoheader;
    if ((bitmapinfoheader = (BITMAPINFOHEADER*) malloc(sizeof(BITMAPINFOHEADER))) == NULL)
        printf("erreur allocation mémoire\n");
    unsigned char *img = LoadBitmapFile("pict.bmp", bitmapinfoheader);
    for (int i = 0; i < 20; i++)
    {
        printf("%x \n", img[i]);
    }
    printf("end of run\n");
    return 1;
}