#ifndef EXE_IO_H
#define EXE_IO_H

#include <stdio.h>
#include <stdlib.h>

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
  fread(buff, size, 1, file);

  // null-terminate the buffer
  buff[size] = '\0';

  fclose(file);

  return buff;
}

#endif // EXE_IO_H