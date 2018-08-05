/**
* conf.h/conf.c
* Config file loader/parser, loads a file containg "<key> <value>" pairs.
* Stores them in an array of conf_var_t, has helper functions for easy 
* config var queries.
*/

#ifndef EXE_CONF_H
#define EXE_CONF_H

#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "exe_io.h"

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

typedef struct {
  conf_var_t *vars;
  int length, success;
} conf_t;

static char *conf_get_string(conf_t *conf, const char *key);
static int conf_get_int(conf_t *conf, const char *key);
static conf_var_t *conf_get_var(conf_t *conf, const char *key);
static inline int conf_is_number(const char *str) {
  if (str == NULL || *str == '\0' || isspace(*str))
    return 0;

  char *p;
  strtod(str, &p);
  return *p == '\0';
}

/**
 * [conf_load loads a <key> <var> config file]
 * @param conf [conf_t pointer]
 * @param path [config file path]
 */
static int conf_load(conf_t *conf, const char *path)
{
  conf->success = 0;
  printf("Loading config file %s\n", path);

  // read config file contents
  char *buff = io_read_file(path, "r", NULL);
  if (buff == NULL)
    return 0;

  // duplicate buff contents
  char str[strlen(buff)];
  strcpy(str, buff);

  // get token count
  int t_count = 0;
  char *token = strtok(str, " \n");
  while (token) {
    t_count++;
    token = strtok(NULL, " \n");
  }

  // should be divisible by 2
  if (t_count % 2) {
    printf("Syntax error in config file %s\n", path);
    printf("Config key is missing a value\n");
    free(buff);
    return 0;
  }

  // allocate space for vars
  conf->length  = t_count/2;
  conf->vars    = malloc(sizeof(conf_var_t)*conf->length);

  // parse tokens
  int i = 0, t = 0;
  strcpy(str, buff);
  token = strtok(str, " \t\n");
  while (token) {
    if (!t) {
      // its a key
      strcpy(conf->vars[i].key, token);
    } else {
      // its a var
      if (conf_is_number(token)) {
        conf->vars[i].i = atoi(token);
        conf->vars[i].type = conf_type_int;
      } else {
        strcpy(conf->vars[i].s, token);
        conf->vars[i].type = conf_type_string;
      }
      
      i++;
    }

    t = !t;

    // get next token
    token = strtok(NULL, " \t\n");
  }

  // woop
  conf->success = 1;

  // debug print out config vars
  printf("Config variables: \n");
  for (i=0; i<conf->length; i++) {
    printf("%s = ", conf->vars[i].key);
    switch (conf->vars[i].type) {
      case conf_type_string:
        printf("%s\n", conf_get_string(conf, conf->vars[i].key));
        break;
      case conf_type_int:
        printf("%i\n", conf_get_int(conf, conf->vars[i].key));
        break;
    }
  }

  free(buff);
  return 1;
}

/**
 * [conf_get_var gets a conf_var_t by the given key]
 * @param conf [conf_t pointer]
 * @param  key [config key value]
 * @return     [conf_var_t container]
 */
static conf_var_t *conf_get_var(conf_t *conf, const char *key)
{
  if (!conf->success)
    return NULL;

  for (int i=0; i<conf->length; i++) {
    if (strcmp(conf->vars[i].key, key) == 0)
      return &conf->vars[i];
  }  

  return NULL;
}

/**
 * [conf_get_int gets a int by the given key]
 * @param conf [conf_t pointer]
 * @param  key [config key value]
 * @return     [int value]
 */
static int conf_get_int(conf_t *conf, const char *key)
{
  if (!conf->success)
    return 0;

  conf_var_t *v = conf_get_var(conf, key);
  if (v != NULL && v->type == conf_type_int)
    return v->i;

  return 0;
}

/**
 * [conf_get_string gets a string by the given key]
 * @param conf [conf_t pointer]
 * @param  key [config key value]
 * @return     [string value]
 */
static char *conf_get_string(conf_t *conf, const char *key)
{
  if (!conf->success)
    return " ";

  conf_var_t *v = conf_get_var(conf, key);
  if (v != NULL && v->type == conf_type_string)
    return v->s;

  return " ";
}

/**
 * [conf_free free the malloc]
 * @param conf [conf_t pointer]
 */
static void conf_free(conf_t *conf)
{
  if (!conf->success)
    return;

  free(conf->vars);
}

#endif // EXE_CONF_H
