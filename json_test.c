#include "json.h"

int main()
{
    char *buff = "Bonjour ! \n ";
    int ret = write_in_file("export.txt", buff);

    char filename[] = "example_file.json";
    char *buff2 = (char *)malloc((unsigned long)fsize(filename) + 1);
    ret = read_from_file(filename, buff2);
    printf("%s\n", buff2);

    int size = get_int_in_json(buff2, "size");
    printf("size = %i\n", size);

    char *str = get_str_in_json(buff2, "string");
    printf("str = %s\n", str);

    char *tab = get_tab_in_json(buff2, "arch");
    printf("tab = %s\n", tab);

    char archCNN[size][20];
    for (int i = 0; i < size; i++)
    {
        memcpy(archCNN[i], "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 20);
        char *str_in_tab = get_str_in_tab(tab, i);
        printf("str_in_tab at idx %d = %s\n", i, str_in_tab);
        strcpy(archCNN[i], str_in_tab);
    }

    archCNN;
    return 0;
}