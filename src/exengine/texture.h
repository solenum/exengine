#ifndef TEXTURE_H
#define TEXTURE_H

#define TEXTURE_LOC "data/textures/"

#include <stdint.h>
#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct {
  GLuint id;
  int width, height;
  char name[32];
  uint8_t *data;
} texture_t;

/**
 * [texture_load load a texture from file]
 * @param  file [file path string]
 * @return      [texture var]
 */
texture_t* texture_load(const char *file, int get_data);

#endif // TEXTURE_H