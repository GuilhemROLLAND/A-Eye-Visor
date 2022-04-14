#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int write_in_file(char *filename, char *buffer);
int read_from_file(char *filename, char *buffer);
unsigned long fsize(char *file);

// GETTERS

/**
 * @brief Get the int in json object
 * 
 * @param buffer 
 * @param param 
 * @return int 
 */
int get_int_in_json(char *buffer, char *param);

/**
 * @brief Get the str in json object
 * 
 * @param buffer 
 * @param param 
 * @return char* 
 */
char *get_str_in_json(char *buffer, char *param);

/**
 * @brief Get the tab in json object
 * 
 * @param buffer 
 * @param param 
 * @return char* 
 */
char *get_tab_in_json(char *buffer, char *param);

/**
 * @brief Get the str in tab object
 * 
 * @param tab 
 * @param idx 
 * @return char* 
 */
char *get_str_in_tab(char *tab, int idx);

