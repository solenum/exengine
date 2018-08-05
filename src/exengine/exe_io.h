#ifndef EXE_IO_H
#define EXE_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <physfs.h>

/**
 * [io_read_file reads a file into a char array]
 * @param  path [file path]
 * @param  mode [access mode]
 * @return      [malloc'd char array, remember to free]
 */
static char* io_read_file(const char *path, const char *mode, size_t *len)
{
  if (!PHYSFS_exists(path)) {
    printf("could not load file %s\n", path);
    return NULL;
  }

  // load the file
  PHYSFS_file *file;
  file = PHYSFS_openRead(path);

  // get file length
  size_t size = PHYSFS_fileLength(file);
  if (len)
    *len = size;

  // allocate space for file data
  char *buff = malloc(size+1);

  // read file contents into buffer
  PHYSFS_readBytes(file, buff, size);

  // null-terminate the buffer
  buff[size] = '\0';

  PHYSFS_close(file);

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