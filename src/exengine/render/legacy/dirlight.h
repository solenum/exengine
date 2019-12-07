/*
  This isn't complete yet,
  it doesnt work, turn back!
*/

#ifndef EX_DIRLIGHT_H
#define EX_DIRLIGHT_H

#include "math/mathlib.h"

#include "glad/glad.h"

extern GLuint ex_dir_light_shader;

typedef struct {
  vec3 position, color, cposition, target;
  mat4x4 transform;
  GLuint depth_map, depth_map_fbo, shader;
  int dynamic, update;
} ex_dir_light_t;

void ex_dir_light_init();

ex_dir_light_t* ex_dir_light_new(vec3 pos, vec3 color, int dynamic);

void ex_dir_light_begin(ex_dir_light_t *l);

void ex_dir_light_draw(ex_dir_light_t *l, GLuint shader);

void ex_dir_light_destroy(ex_dir_light_t *l);

#endif // EX_DIRLIGHT_H