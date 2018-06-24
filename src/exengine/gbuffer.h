#ifndef EX_GBUFFER_H
#define EX_GBUFFER_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern GLuint ex_gshader, ex_gmainshader;
extern GLuint gposition, gnormal;

void ex_gbuffer_init();

void ex_gbuffer_first();

void ex_gbuffer_render(GLuint shader);

void ex_gbuffer_destroy();

#endif // EX_GBUFFER_H