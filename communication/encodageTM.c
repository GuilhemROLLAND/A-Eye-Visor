#include "encodageTM.h"


STRINGLENGTH* ackTxtFileReading(char* filePath) 
{
    char buffer[50];
    STRINGLENGTH* str;
    FILE * inputFile = fopen(filePath, "r");
    if (inputFile == NULL)
        printf("impossible d'ouvrir le fichier\n");
    int i = 0;
    while( ! feof(inputFile)) 
    {   
        buffer[i] = fgetc(inputFile);
        i++;
    }
    
    str->length = i;
    str->string = &buffer[0];
    return str;
}

TELEMESURE* stringEncodedTM(STRINGLENGTH* inputString, unsigned char typeOfAck) 
{
    TELEMESURE* tm;
    char string[inputString->length];
    for (int n =0; n<inputString->length ; n++)
    {
        string[n] = *(inputString->string + n);
    }
    char TM[inputString->length + 1];
    switch(typeOfAck) 
    {
        case 0 : //startStopFlag
            tm->codeOp = 0x30;
            break;
        case 1 : //takePct
            tm->codeOp = 0x20;
            break;
        case 2 : //loadWeights
            tm->codeOp = 0x40;
            break;
        case 3 : //arbitrary string
            tm->codeOp = 0x60;
            break;
    }
    char* ptr_tm = &string[0];
    tm->addrContent = ptr_tm;
    tm->length = inputString->length+1;
    return tm;
}

TELEMESURE* imgEncodedTM(unsigned char* addr, int length) 
{
    TELEMESURE* tm;
    tm->codeOp = 0x50;
    tm->addrContent = addr;
    tm->length = length;
    return tm;
}

