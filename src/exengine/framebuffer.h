#ifndef EX_FRAMEBUFFER_H
#define EX_FRAMEBUFFER_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern GLuint ex_fbo_shader;

typedef struct {
  GLuint fbo, rbo, colorbuffer;
  int width, height;
} ex_framebuffer_t;

void ex_framebuffer_init();

ex_framebuffer_t* ex_framebuffer_new(int width, int height);

void ex_framebuffer_bind(ex_framebuffer_t *fb);

void ex_framebuffer_draw(ex_framebuffer_t *fb, int x, int y, int width, int height);

ex_framebuffer_t* ex_framebuffer_resize(ex_framebuffer_t *fb, int width, int height);

void ex_framebuffer_destroy(ex_framebuffer_t *fb);

void ex_framebuffer_cleanup();

#endif // EX_FRAMEBUFFER_H