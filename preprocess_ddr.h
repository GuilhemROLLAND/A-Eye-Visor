#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include <byteswap.h>


#ifndef _PREPROCESS_H
#define _PREPROCESS_H

    //STRUCTURES 
    #ifndef H_BITMAPFILEHEADER
    #define H_BITMAPFILEHEADER
    
    typedef struct __attribute__((__packed__)) __attribute__ ((aligned)) BITMAPFILEHEADER
    {
        unsigned short fType;  //specifies the file type
        unsigned int fSize;  //specifies the size in bytes of the bitmap file
        unsigned short fReserved1;  //reserved; must be 0
        unsigned short fReserved2;  //reserved; must be 0
        unsigned int fOffBits;  //specifies the offset in bytes from the bitmapfileheader to the bitmap bits
    }BITMAPFILEHEADER;

    #endif

    #ifndef H_BITMAPINFOHEADER
    #define H_BITMAPINFOHEADER

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

    #endif

    // FUNCTIONS

    /**
     * @brief 
     * Take an address of a starting image in DDR (theorically) and extract informations from BMP 
     * header to corresponding structure. Place the image data into an array of corresponding size
     * @param addr starting address of a BMP file
     * @return bmpImg array containing the image data as [px1R,px1G,px1B,px2R,px2G,px2B] 
     */
    unsigned char* readImgBmp(unsigned char* addr);

    /**
     * @brief this function crop the image on the side to have a squared shaped image.
     * 
     * @param bmpImg array of the input image samples stored as [pxR1,pxG1,pxB1,pxR2,pxG2,pxB2, ...]
     * @param width width of the input image
     * @param height height of the input image
     * @param pixelsToCrop sum of cropping on both sides
     * @return unsigned* pointer to the resized image
     */
    unsigned char* resizeImg(unsigned char* bmpImg, unsigned char width, unsigned char length, unsigned char pixelsToCrop);

    /**
     * @brief This function performs an average pooling on an img stored as [pxR1,pxG1,pxB1,pxR2,pxG2,pxB2, ...]
     * 
     * @param img array containing samples of the images stored as [pxR1,pxG1,pxB1,pxR2,pxG2,pxB2, ...]
     * @param height height of the image
     * @param poolingLength size of the pooling filter. always square shaped (if pooling is 2, kernel is 2*2)
     * @return unsigned char* pointer to the pooled img. 
     */
    unsigned char* avgPooling(unsigned char* img, unsigned char width, unsigned char height, unsigned char poolingLength);

    /**
     * @brief Performs a rescaling on the pixels value. Change values from [0;255] to [0;1]
     * 
     * @param img input img stored as [pxR1,pxG1,pxB1,pxR2,pxG2,pxB2, ...]
     * @param height height of the image
     * @param width width of the image. must be *3 if the image is RGB 
     * @return float* 
     */
    float* rescaling(unsigned char* img, unsigned char height, unsigned char width);

#endif