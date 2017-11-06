#ifndef EX_FRAMEBUFFER_H
#define EX_FRAMEBUFFER_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern GLuint fbo_shader;

/**
 * [framebuffer_init setup framebuffers and screen quad]
 */
void framebuffer_init();

/**
 * [framebuffer_first call before first rendering pass]
 */
void framebuffer_first();

/**
 * [framebuffer_render_quad renders the screen quad]
 */
void framebuffer_render_quad();

void framebuffer_destroy();

#endif // EX_FRAMEBUFFER_H