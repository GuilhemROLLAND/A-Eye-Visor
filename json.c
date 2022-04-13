#include "json.h"
/**
 * @brief Write in a file the content of the buffer
 *
 * @param filename name of the file. If exist, will be overwrite
 * @param buffer string buffer with the content
 * @return int Number of characters written
 */
int write_in_file(char *filename, char *buffer)
{
    FILE *file = fopen(filename, "w+");
    int ret = fprintf(file, "%s", buffer);
    fclose(file);
    return ret;
}

/**
 * @brief Write in a buffer one line of a file
 * 
 * @param filename name of the file
 * @param buffer string buffer with the content
 * @return int Number of characters read
 */
int read_from_file(char *filename, char *buffer)
{
    FILE *file = fopen(filename, "r");
    fgets(buffer,30,file);
    // int ret = fscanf(file, "%s", buffer);
    return 0;
}



int main()
{
    char * buff = "Bonjour ! \n";
    int ret = write_in_file("export.txt", buff);

    char tab[30];
    char * buff2 = tab;
    ret = read_from_file("export.txt", buff2);
    printf("%s", buff2);
    return 0;
}