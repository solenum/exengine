#ifndef EX_CACHE_H
#define EX_CACHE_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "model.h"

void ex_cache_init();

void ex_cache_model(ex_model_t *m);

ex_model_t* ex_cache_get_model(const char *path);

GLuint ex_cache_texture(const char *file);

void ex_cache_flush();

#endif // EX_CACHE_H