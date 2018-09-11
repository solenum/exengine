/* ssao
  A half-kernel ssao implementation,
  pretty performance heavy.
*/

#ifndef EX_SSAO_H
#define EX_SSAO_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define SSAO_NUM_SAMPLES 32

/**
 * [ssao_init init the ssao module]
 */
void ssao_init();

/**
 * [ssao_render set as render target]
 * @param projection [cameras projection matrix]
 * @param view       [cameras view matrix]
 */
void ssao_render(mat4x4 projection, mat4x4 view);

/**
 * [ssao_bind_texture]
 * @param shader [shader to use]
 */
void ssao_bind_texture(GLuint shader);

/**
 * [ssao_bind_default binds an empty ssao texture]
 * @param shader [shader to use]
 */
void ssao_bind_default(GLuint shader);

#endif // EX_SSAO_H