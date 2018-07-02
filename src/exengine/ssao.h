#ifndef EX_SSAO_H
#define EX_SSAO_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define SSAO_NUM_SAMPLES 32

void ssao_init();

void ssao_render(mat4x4 projection, mat4x4 view);

void ssao_bind_texture(GLuint shader);

#endif // EX_SSAO_H