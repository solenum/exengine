/**
* conf.h/conf.c
* Config file loader/parser, loads a file containg "<key> <value>" pairs.
* Stores them in an array of conf_var_t, has helper functions for easy 
* config var queries.
*/

#ifndef CONF_H
#define CONF_H

#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
  conf_type_undefined,
  conf_type_int,
  conf_type_string
} conf_type_e;

typedef struct {
  char key[256];
  conf_type_e type;
  union {
    uint32_t i;
    char s[256];
  };
} conf_var_t;

extern int conf_length;
extern conf_var_t *conf;

static inline int conf_is_number(const char *str) {
  if (str == NULL || *str == '\0' || isspace(*str))
    return 0;

  char *p;
  strtod(str, &p);
  return *p == '\0';
}

/**
 * [conf_load loads a <key> <var> config file]
 * @param path [config file path]
 */
void conf_load(const char *path);

/**
 * [conf_get_var gets a conf_var_t by the given key]
 * @param  key [config key value]
 * @return     [conf_var_t container]
 */
conf_var_t *conf_get_var(const char *key);

/**
 * [conf_get_int gets a int by the given key]
 * @param  key [config key value]
 * @return     [int value]
 */
int conf_get_int(const char *key);

/**
 * [conf_get_string gets a string by the given key]
 * @param  key [config key value]
 * @return     [string value]
 */
char *conf_get_string(const char *key);

/**
 * [conf_free cleans up data]
 */
void conf_free();

#endif // CONF_H