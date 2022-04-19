#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef _ENCODAGETM_H
#define _ENCODAGETM_H

    //STRUCTURES


    #ifndef H_TELEMESURE
    #define H_TELEMESURE

    typedef struct
    {
        int length;
        char* addrContent; 
        char codeOp;
    }TELEMESURE;
    #endif

    #ifndef H_CHANGEMODEACK
    #define H_CHANGEMODEACK

    typedef enum 
    {
        startStopFlag,
        takePctFlag,
        loadWeigthsFlag,
        stringFlag,
        numberofAck
    }CHANGEMODEACK;
    #endif

    #ifndef H_TYPEOFTM
    #define H_TYPEOFTM

    typedef struct 
    {
        bool imgFlag;
        CHANGEMODEACK ackFlag; 
    }TYPEOFTM;
    #endif

    #ifndef H_STRINGLENGTH
    #define H_STRINGLENGTH

    typedef struct
    {
        unsigned char length;
        char* string; 
    }STRINGLENGTH;
    #endif

    // FUNCTIONS


    /**
     * @brief read a txt file containing the ack and outputs the ack in string format
     * 
     * @param filePath tct file containing the ack
     * @return char* ack in a string format
     */
    STRINGLENGTH* ackTxtFileReading(char* filePath);

    /**
     * @brief convert a string into a telemesure
     * 
     * @param inputString input string with the length
     * @param typeOfAck ack type
     * @return TELEMESURE* struct containing the starting addr of the data, codeOp and length
     */
    TELEMESURE* stringEncodedTM(STRINGLENGTH* inputString, unsigned char typeOfAck);

    /**
     * @brief convert an img into a telemesure
     * 
     * @param addr starting address of the img
     * @param length length of the img
     * @return TELEMESURE* struct containing the starting addr of the data, codeOp and length
     */
    TELEMESURE* imgEncodedTM(unsigned char* addr, int length);

#endif