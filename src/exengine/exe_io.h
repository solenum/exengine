#ifndef EXE_IO_H
#define EXE_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * [io_read_file reads a file into a char array]
 * @param  path [file path]
 * @param  mode [access mode]
 * @return      [malloc'd char array, remember to free]
 */
static char* io_read_file(const char *path, const char *mode)
{
  // load the file
  FILE *file;
  file = fopen(path, mode);
  if (file == NULL) {
    printf("could not load file %s\n", path);
    return NULL;
  }

  // get file length
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  // allocate space for file data
  char *buff = malloc(size+1);

  // read file contents into buffer
  size_t r = fread(buff, size, 1, file);

  // null-terminate the buffer
  buff[size] = '\0';

  fclose(file);

  return buff;
}


/**
 * [io_prefix_str prefix a string with another]
 * @param  dest   [destination string]
 * @param  src    [input string]
 * @param  prefix [prefix string]
 * @return        [input string with prefix]
 */
static inline void io_prefix_str(char *dest, const char *src, const char *prefix)
{
  if (src == NULL) {
    dest[0] = '\0';
    return;
  }

  size_t len = strlen(prefix);
  strcpy(dest, prefix);
  strcpy(&dest[len], src);
}

#endif // EXE_IO_H