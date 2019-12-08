/* ini
  A simple ini style config loader, strips all
  white-space from the input file upon loading.

  Supports two variable types, strings and floats,
  make sure you cast values to your required type.
  key=value pairs *must* be in a [section].
  
  File extension doesn't matter, as long as its a
  text file and not a binary.
  
  Example conf.ini:

  # [graphics]
  # window_width = 1280
  # quality = high
  #
  # [input]
  # bind_left = a
*/
#ifndef EX_INI_H
#define EX_INI_H

#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
  ex_ini_type_undefined,
  ex_ini_type_string,
  ex_ini_type_float,
} ex_ini_type_e;

typedef struct {
  char key[256];
  ex_ini_type_e type;
  union {
    char s[256];
    float f;
  };
} ex_ini_var_t;

typedef struct {
  ex_ini_var_t vars[256];
  char name[256];
  int length;
} ex_ini_section_t;

typedef struct {
  ex_ini_section_t sections[256];
  int length, success;
} ex_ini_t;

/**
 * [ex_ini_load load or update from an ini file]
 * @param  ini  [ini instance to use]
 * @param  path [file path]
 * @return      [non-zero upon success]
 */
int ex_ini_load(ex_ini_t *ini, const char *path);

/**
 * [ex_ini_save saves the ini structure to a file]
 * @param ini  [ini to save]
 * @param path [file path]
 */
void ex_ini_save(ex_ini_t *ini, const char *path);

/**
 * [ex_ini_get_var get a key-value variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [variable struct]
 */
ex_ini_var_t *ex_ini_get_var(ex_ini_t *ini, const char *sec, const char *key);

/**
 * [ex_ini_get_string get a string variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [string pointer]
 */
char *ex_ini_get_string(ex_ini_t *ini, const char *sec, const char *key);

/**
 * [ex_ini_get_float get a float variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [float value]
 */
float ex_ini_get_float(ex_ini_t *ini, const char *sec, const char *key);

/**
 * [ex_ini_set_string set a string variable]
 * @param ini   [ini instance to use]
 * @param sec   [variable section]
 * @param key   [variable key]
 * @param value [string value]
 */
void ex_ini_set_string(ex_ini_t *ini, const char *sec, const char *key, const char *value);

/**
 * [ex_ini_set_float set a float variable]
 * @param ini   [ini instance to use]
 * @param sec   [variable section]
 * @param key   [variable key]
 * @param value [float value]
 */
void ex_ini_set_float(ex_ini_t *ini, const char *sec, const char *key, const float value);

#endif // EX_INI_H