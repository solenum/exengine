#ifndef EX_FRAMEBUFFER_H
#define EX_FRAMEBUFFER_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern GLuint ex_fbo_shader;

/**
 * [ex_framebuffer_init setup framebuffers and screen quad]
 */
void ex_framebuffer_init();

/**
 * [ex_framebuffer_first call before first rendering pass]
 */
void ex_framebuffer_first();

/**
 * [ex_framebuffer_render_quad renders the screen quad]
 */
void ex_framebuffer_render_quad();

void ex_framebuffer_destroy();

#endif // EX_FRAMEBUFFER_H