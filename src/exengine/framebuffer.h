/* framebuffer
  A generic framebuffer, essentially
  render to texture.  Often referred to
  as a canvas.

  Mostly used internally by the scene module.
*/

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

/**
 * [ex_framebuffer_init init the framebuffer module]
 */
void ex_framebuffer_init();

/**
 * [ex_framebuffer_new defines a new framebuffer]
 * @param  width  []
 * @param  height []
 * @return        [the new framebuffer]
 */
ex_framebuffer_t* ex_framebuffer_new(int width, int height);

/**
 * [ex_framebuffer_bind set as the render target]
 * @param fb [framebuffer to bind]
 */
void ex_framebuffer_bind(ex_framebuffer_t *fb);

/**
 * [ex_framebuffer_draw render the framebuffer to the screen]
 * @param fb     [framebuffer to render]
 * @param x      [x offset]
 * @param y      [y offset]
 * @param width  [render width]
 * @param height [render height]
 */
void ex_framebuffer_draw(ex_framebuffer_t *fb, int x, int y, int width, int height);

/**
 * [ex_framebuffer_resize resize the framebuffers buffers]
 * @param  fb     [framebuffer to resize]
 * @param  width  [new width]
 * @param  height [new height]
 * @return        [the resized framebuffer]
 */
ex_framebuffer_t* ex_framebuffer_resize(ex_framebuffer_t *fb, int width, int height);

/**
 * [ex_framebuffer_destroy clean up a framebuffers data]
 * @param fb [framebuffer to destroy]
 */
void ex_framebuffer_destroy(ex_framebuffer_t *fb);

/**
 * [ex_framebuffer_cleanup cleanup framebuffer module data]
 */
void ex_framebuffer_cleanup();

#endif // EX_FRAMEBUFFER_H