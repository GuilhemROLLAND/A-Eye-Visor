#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int write_in_file(char *filename, char *buffer);
int read_from_file(char *filename, char *buffer);
unsigned long fsize(char *file);

/**
 * @brief
 *
 * @param ptrToFloat
 * @param nbrFloat
 * @return char*
 */
char *write_float_in_buffer(float *ptrToFloat, int nbrFloat);

/**
 * @brief 
 * 
 * @param filename 
 * @param ptrToFloat 
 * @param nbrFloat 
 * @return int 
 */
int write_float_in_file(char *filename, float *ptrToFloat, int nbrFloat);

/**
 * @brief forward the pointer to the beginning of the number
 *
 * @param str
 * @return char*
 */
char *go_to_number(char *str);

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
 * @brief Get the object in json object
 *
 * @param buffer
 * @param param
 * @return char*
 */
char *get_object_in_json(char *buffer, char *param);

/**
 * @brief Get the str in tab object
 *
 * @param tab
 * @param idx
 * @return char*
 */
char *get_str_in_tab(char *tab, int idx);

/**
 * @brief Get the tab in tab with idx object
 *
 * @param tab
 * @param idx
 * @return char*
 */
char *get_tab_in_tab(char *tab, int idx);

/**
 * @brief Get the size of float object
 *
 * @param str
 * @return int
 */
int get_size_of_float(char *str);

/**
 * @brief Get the size of int object
 *
 * @param str
 * @return int
 */
int get_size_of_int(char *str);

/**
 * @brief Get the float in string object
 *
 * @param str
 * @param idx
 * @return float
 */
float get_float_in_string(char *str, int idx);

/**
 * @brief Get the int in string object
 *
 * @param str
 * @return int
 */
int get_int_in_string(char *str, int idx);