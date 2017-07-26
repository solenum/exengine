#ifndef EX_GBUFFER_H
#define EX_GBUFFER_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern GLuint gshader, gmainshader;

void gbuffer_init();

void gbuffer_first();

void gbuffer_render(GLuint shader);

void gbuffer_destroy();

#endif // EX_GBUFFER_H