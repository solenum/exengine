#include "ini.h"
#include "exe_io.h"

int ini_load(ini_t *ini, const char *path)
{
  printf("Loading config file %s\n", path);

  // read config file contents
  char *buff = io_read_file(path, "r", NULL);
  if (buff == NULL)
    return 0;

  // strip white-space
  int j = 0;
  for (int i=0; i<strlen(buff); i++)
    if (buff[i] != ' ' && buff[i] != '\t')
      buff[j++] = buff[i];
  buff[j] = '\0';

  // tokenize buffer
  int cur_section = -1;
  char *token = strtok(buff, "\t\n");
  while (token) {
    // check if section
    if (token[0] == '[' && token[strlen(token)-1] == ']') {
      char section[256] = {0};
      strncpy(section, &token[1], strlen(token)-2);

      // attempt set active section
      int i;
      for (i=0; i<ini->length; i++) {
        if (strcmp(ini->sections[i].name, section) == 0) {
          cur_section = i;
          break;
        }
      }

      // no section found, make it
      if (i == ini->length) {
        strcpy(ini->sections[ini->length].name, section);
        ini->sections[ini->length].length = 0;
        cur_section = ini->length;
        ini->length++;
      }
    }

    // get keys and values
    char *e, *key, *value;
    if ((e = strstr(token, "="))) {
      key = token;
      value = &e[1];
      e[0] = '\0';
      
      if (cur_section > -1) {
        ini_section_t *section = &ini->sections[cur_section];
        
        // check to see if key already exists
        ini_var_t *cur_var = NULL;
        for (int i=0; i<section->length; i++) {
          // key found
          if (strcmp(section->vars[i].key, key) == 0) {
            cur_var = &section->vars[i];
            break;
          }
        }
        
        // no key found, make it
        if (!cur_var) {
          strcpy(section->vars[section->length].key, key);
          cur_var = &section->vars[section->length];
          section->length++;
        }

        // insert key-value pair into section
        if (cur_var) {
          // is it a number?
          char *p;
          strtod(value, &p);
          if (*p == '\0') {
            cur_var->f = strtof(value, NULL);
            cur_var->type = ini_type_float;
          } else {
            strcpy(cur_var->s, value);
            cur_var->type = ini_type_string;
          }
        }
      }
    }

    token = strtok(NULL, "\t\n");
  }

  /*for (int i=0; i<ini->length; i++) {
    ini_section_t *section = &ini->sections[i];
    for (int j=0; j<section->length; j++) {
      ini_var_t *var = &section->vars[j];
      switch (var->type) {
        case ini_type_string: {
          printf("%s[%s] = %s\n", section->name, var->key, var->s);
          break;
        }
        case ini_type_float: {
          printf("%s[%s] = %f\n", section->name, var->key, var->f);
          break;
        }
        case ini_type_undefined: {
          break;
        }
      }
    }
  }*/

  free(buff);
  return 1;
}

ini_var_t *ini_get_var(ini_t *ini, const char *sec, const char *key)
{
  for (int i=0; i<ini->length; i++) {
    ini_section_t *section = &ini->sections[i];

    if (strcmp(section->name, sec) != 0)
      continue;

    for (int j=0; j<section->length; j++) {
      ini_var_t *var = &section->vars[j];
      if (strcmp(var->key, key) == 0) {
        return var;
      }
    }
  }

  return NULL;
}

char *ini_get_string(ini_t *ini, const char *sec, const char *key)
{
  ini_var_t *var = ini_get_var(ini, sec, key);

  if (var && var->type == ini_type_string)
    return var->s;

  return "";
}

float ini_get_float(ini_t *ini, const char *sec, const char *key)
{
  ini_var_t *var = ini_get_var(ini, sec, key);

  if (var && var->type == ini_type_float)
    return var->f;

  return 0.0f;
}