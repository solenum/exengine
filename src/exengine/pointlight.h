#ifndef EX_POINTLIGHT_H
#define EX_POINTLIGHT_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>

#define EX_POINT_FAR_PLANE 60
#define EX_POINT_SHADOW_DIST 150

typedef struct {
  vec3 position, color;
  mat4x4 transform[6];
  GLuint depth_map, depth_map_fbo, shader;
  int dynamic, update, is_shadow, is_visible;
  float distance_to_cam;
} ex_point_light_t;

void ex_point_light_init();

ex_point_light_t *ex_point_light_new(vec3 pos, vec3 color, int dynamic);

void ex_point_light_begin(ex_point_light_t *l);

void ex_point_light_draw(ex_point_light_t *l, GLuint shader, const char *prefix);

void ex_point_light_destroy(ex_point_light_t *l);

#endif // EX_POINTLIGHT_H