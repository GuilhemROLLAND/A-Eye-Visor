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
 * @brief Write in a buffer the content of a file
 * 
 * @param filename name of the file
 * @param buffer string buffer with the content
 * @return int Number of characters read
 */
int read_from_file(char *filename, char *buffer)
{
    char *data;
    int len = 0;
    if (access(filename, F_OK) == 0)
    {
        data = (char *)malloc((unsigned long)fsize(filename) + 1);
        FILE *fp;
        fp = fopen(filename, "r");
        while (fgets(buffer, 1000000, fp))
        {
            len += sprintf(data + len, "%s", buffer);
        }
        fclose(fp);
    }
    else
    {
        printf("ERROR: File %s not found.\n", filename);
    }
    return len;
}

unsigned long fsize(char *file)
{
    FILE *f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(f);
    fclose(f);
    return len;
}

int main()
{
    char * buff = "Bonjour ! sdasd \n sadad ";
    int ret = write_in_file("export.txt", buff);

    char tab[30];
    char * buff2 = tab;
    ret = read_from_file("export.txt", buff2);
    printf("%s", buff2);
    return 0;
}