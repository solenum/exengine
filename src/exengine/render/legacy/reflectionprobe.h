/*
  This isn't complete yet,
  it doesnt work, turn back!
*/

#ifndef EX_REFLECTION_H
#define EX_REFLECTION_H

#include "math/mathlib.h"

#include "glad/glad.h"

typedef struct {
  vec3 position;
  mat4x4 transform[6];
  GLuint color_buffer, depth_buffer, fbo, shader;
  int update;
} ex_reflection_t;

void ex_reflection_init();

ex_reflection_t *ex_reflection_new(vec3 position);

void ex_reflection_begin(ex_reflection_t *r);

void ex_reflection_draw(ex_reflection_t *r, GLuint shader);

void ex_reflection_destroy(ex_reflection_t *r);

#endif // EX_REFLECTION_H