#ifndef DIRLIGHT_H
#define DIRLIGHT_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>

extern GLuint dir_light_shader;

typedef struct {
  vec3 position, color, cposition, target;
  mat4x4 transform;
  GLuint depth_map, depth_map_fbo, shader;
  int dynamic, update;
} dir_light_t;

void dir_light_init();

dir_light_t* dir_light_new(vec3 pos, vec3 color, int dynamic);

void dir_light_begin(dir_light_t *l);

void dir_light_draw(dir_light_t *l, GLuint shader);

void dir_light_destroy(dir_light_t *l);

#endif // DIRLIGHT_H