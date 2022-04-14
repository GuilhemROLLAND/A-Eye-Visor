#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int write_in_file(char *filename, char *buffer);
int read_from_file(char *filename, char *buffer);
unsigned long fsize(char *file);
int get_int_json(char *buffer, char *param);


// TO DO
int read_json(char *buffer);
