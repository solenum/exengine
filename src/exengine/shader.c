#include "shader.h"
#include <string.h>

GLint ex_uniform_map[256][256] = {0};
GLint ex_uniform_locations[256][256] = {0};

inline GLint ex_uniform(GLuint shader, const char *str)
{
  const char *string = str;
  uint32_t key = 5381;
  int c;

  // hash * 33 + c
  while (c = *str++)
    key = ((key << 5) + key) + c;

  // check if location cached already
  int i=0;
  for (i=0; i<256; i++) {
    // end of array
    if (!ex_uniform_map[shader][i])
      break;

    // check cached
    if (ex_uniform_map[shader][i] == key)
      return ex_uniform_locations[shader][i];
  }

  // store and return it
  GLint value = glGetUniformLocation(shader, string);
  ex_uniform_map[shader][i] = key;
  ex_uniform_locations[shader][i] = value;

  return value;
}