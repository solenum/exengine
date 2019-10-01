/* gbuffer
  The buffer used for deferred rendering.

  Combines positions, normals, colors,
  and specularity into a single framebuffer.

  Geometry is rendered to this once, and
  the resulting buffer is used for each lighting
  pass.  Cutting down the cost of lighting
  dramatically.
*/

#ifndef EX_GBUFFER_H
#define EX_GBUFFER_H

#include "glad/glad.h"
#include <SDL2/SDL.h>

extern GLuint ex_gshader, ex_gmainshader;
extern GLuint gposition, gnormal;

/**
 * [ex_gbuffer_init init the gbuffer module]
 * @param reinit [1 if reinitializing an existing gbuffer]
 */
void ex_gbuffer_init(int reinit);

/**
 * [ex_gbuffer_first set as render target]
 * @param x  [x offset]
 * @param y  [y offset]
 * @param vw [render width]
 * @param vh [render height]
 */
void ex_gbuffer_first(int x, int y, int vw, int vh);

/**
 * [ex_gbuffer_render draw gbuffer to screen]
 * @param shader [shader to use]
 */
void ex_gbuffer_render(GLuint shader);

/**
 * [ex_gbuffer_resize resize internal buffers]
 * @param width  [new width]
 * @param height [new height]
 */
void ex_gbuffer_resize(int width, int height);

/**
 * [ex_gbuffer_destroy cleanup gbuffer data]
 */
void ex_gbuffer_destroy();

#endif // EX_GBUFFER_H