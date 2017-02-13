#include <conf.h>
#include <io.h>
#include <stdio.h>

int conf_length;
conf_var_t *conf;

void conf_load(const char *path)
{
  printf("Loading config file %s\n", path);

  // read config file contents
  char *buff = io_read_file(path, "r");
  if (buff == NULL)
    return;

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
    free(buff);
    return;
  }

  // allocate space for vars
  conf_length = t_count/2;
  conf = malloc(sizeof(conf_var_t)*conf_length);

  // parse tokens
  int i = 0, t = 0;
  strcpy(str, buff);
  token = strtok(str, " \t\n");
  while (token) {
    if (!t) {
      // its a key
      strcpy(conf[i].key, token);
    } else {
      // its a var
      if (conf_is_number(token)) {
        conf[i].i = atoi(token);
        conf[i].type = conf_type_int;
      } else {
        strcpy(conf[i].s, token);
        conf[i].type = conf_type_string;
      }
      
      i++;
    }

    t = !t;

    // get next token
    token = strtok(NULL, " \t\n");
  }

  // print out config vars
  printf("Config variables: \n");
  for (i=0; i<conf_length; i++) {
    printf("%s = ", conf[i].key);
    switch (conf[i].type) {
      case conf_type_string:
        printf("%s\n", conf_get_string(conf[i].key));
        break;
      case conf_type_int:
        printf("%i\n", conf_get_int(conf[i].key));
        break;
    }
  }

  free(buff);
}

conf_var_t *conf_get_var(const char *key)
{
  for (int i=0; i<conf_length; i++) {
    if (strcmp(conf[i].key, key) == 0)
      return &conf[i];
  }  

  return NULL;
}

int conf_get_int(const char *key)
{
  conf_var_t *v = conf_get_var(key);
  if (v != NULL && v->type == conf_type_int)
    return v->i;

  return 0;
}

char *conf_get_string(const char *key)
{
  conf_var_t *v = conf_get_var(key);
  if (v != NULL && v->type == conf_type_string)
    return v->s;

  return " ";
}

void conf_free()
{
  free(conf);
}