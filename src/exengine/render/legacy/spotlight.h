/*
  This isn't complete yet,
  it doesnt work, turn back!
*/

#ifndef EX_SPOTLIGHT_H
#define EX_SPOTLIGHT_H

#include "math/mathlib.h"

#include "glad/glad.h"

#define EX_SPOT_FAR_PLANE 100
#define EX_SPOT_SHADOW_DIST 150

typedef struct {
  vec3 position, color, direction;
  mat4x4 transform;
  GLuint depth_map, depth_map_fbo, shader;
  int dynamic, update, is_shadow, is_visible;
  float distance_to_cam, inner, outer;
} ex_spot_light_t;

void ex_spot_light_init();

ex_spot_light_t* ex_spot_light_new(vec3 pos, vec3 color, int dynamic);

void ex_spot_light_begin(ex_spot_light_t *l);

void ex_spot_light_draw(ex_spot_light_t *l, GLuint shader, const char *prefix);

void ex_spot_light_destroy(ex_spot_light_t *l);


#endif // EX_SPOTLIGHT_H