#ifndef EX_SPOTLIGHT_H
#define EX_SPOTLIGHT_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>

#define SPOT_FAR_PLANE 100
#define SPOT_SHADOW_DIST 150

typedef struct {
  vec3 position, color, direction;
  mat4x4 transform;
  GLuint depth_map, depth_map_fbo, shader;
  int dynamic, update, is_shadow, is_visible;
  float distance_to_cam, inner, outer;
} spot_light_t;

void spot_light_init();

spot_light_t* spot_light_new(vec3 pos, vec3 color, int dynamic);

void spot_light_begin(spot_light_t *l);

void spot_light_draw(spot_light_t *l, GLuint shader, const char *prefix);

void spot_light_destroy(spot_light_t *l);


#endif // EX_SPOTLIGHT_H