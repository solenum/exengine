#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>

typedef struct {
  vec3 position;
  vec3 color;
  float intensity;
  mat4x4 transform[6];
  GLuint depth_map, depth_map_fbo, shader;
} point_light_t;

void point_light_init();

point_light_t *point_light_new(vec3 pos, vec3 color, float intensity);

void point_light_begin(point_light_t *l);

void point_light_draw(point_light_t *l, GLuint shader);

void point_light_destroy(point_light_t *l);

#endif // POINTLIGHT_H