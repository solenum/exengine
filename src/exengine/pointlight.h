/* pointlight
  Defines both the shadow casting
  and non shadow casting pointlights.

  Shadows are cast using geometry shaders
  to reduce the amount of draw calls per
  light source.
*/

#ifndef EX_POINTLIGHT_H
#define EX_POINTLIGHT_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>

#define EX_POINT_FAR_PLANE 60
#define EX_POINT_SHADOW_DIST 150

/*
  The shadow map resolution, this
  needs to be changed so that the user
  can specify a different resolution
  without modifying this header.
*/
#define SHADOW_MAP_SIZE 1024

typedef struct {
  vec3 position, color;
  mat4x4 transform[6];
  GLuint depth_map, depth_map_fbo, shader;
  int dynamic, update, is_shadow, is_visible;
  float distance_to_cam;
} ex_point_light_t;

/**
 * [ex_point_light_init init the pointlight module]
 */
void ex_point_light_init();

/**
 * [ex_point_light_new defines a new pointlight]
 * @param  pos     [the initial position]
 * @param  color   [the initial color]
 * @param  dynamic [1 if the shadows are dynamic]
 * @return         [the new pointlight]
 */
ex_point_light_t *ex_point_light_new(vec3 pos, vec3 color, int dynamic);

/**
 * [ex_point_light_begin set as rendertarget]
 * @param l [pointlight to use]
 */
void ex_point_light_begin(ex_point_light_t *l);

/**
 * [ex_point_light_draw render the depth map, light values etc]
 * @param l      [pointlight to use]
 * @param shader [shader to use]
 * @param prefix [used for non shadowcasting sources]
 */
void ex_point_light_draw(ex_point_light_t *l, GLuint shader, const char *prefix, int deferred);

/**
 * [ex_point_light_destroy cleanup a pointlights data]
 * @param l [the pointlight to destroy]
 */
void ex_point_light_destroy(ex_point_light_t *l);

#endif // EX_POINTLIGHT_H