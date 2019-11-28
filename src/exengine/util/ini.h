#ifndef EX_INI_H
#define EX_INI_H

#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
  ini_type_undefined,
  ini_type_string,
  ini_type_float,
} ini_type_e;

typedef struct {
  char key[256];
  ini_type_e type;
  union {
    char s[256];
    float f;
  };
} ini_var_t;

typedef struct {
  ini_var_t vars[256];
  char name[256];
  int length;
} ini_section_t;

typedef struct {
  ini_section_t sections[256];
  int length, success;
} ini_t;

/**
 * [ini_load load or update from an ini file]
 * @param  ini  [ini instance to use]
 * @param  path [file path]
 * @return      [non-zero upon success]
 */
int ini_load(ini_t *ini, const char *path);

/**
 * [ini_get_var get a key-value variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [variable struct]
 */
ini_var_t *ini_get_var(ini_t *ini, const char *sec, const char *key);

/**
 * [ini_get_string get a string variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [string pointer]
 */
char *ini_get_string(ini_t *ini, const char *sec, const char *key);

/**
 * [ini_get_float get a float variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [float value]
 */
float ini_get_float(ini_t *ini, const char *sec, const char *key);

#endif // EX_INI_H