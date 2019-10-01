/* collision
  Handles the collision checking between
  an ellipsoid and a triangle.

  Based on Kasper Fauerbys's paper.
  http://www.peroxide.dk/papers/collision/collision.pdf
*/

#ifndef EX_COLLISION_H
#define EX_COLLISION_H

#include "math/mathlib.h"

typedef struct {
  vec3 origin;
  vec3 normal;
  float equation[4];
} ex_plane_t;

typedef struct {
  // r3 space
  vec3 r3_velocity, r3_position;

  // ellipsoid space
  vec3 e_radius;
  vec3 e_velocity;
  vec3 e_norm_velocity;
  vec3 e_base_point;

  // original tri points
  vec3 a,b,c;

  // hit information
  int found_collision;
  float nearest_distance;
  double t;
  vec3 intersect_point;
  ex_plane_t plane;

  // iteration depth
  int depth;
} ex_coll_packet_t;

/**
 * [ex_plane_new defines a plane from a origin and normal]
 * @param  a [plane origin]
 * @param  b [plane normal]
 * @return   [the generated plane]
 */
ex_plane_t ex_plane_new(const vec3 a, const vec3 b);

/**
 * [ex_triangle_to_plane define a plane from a triangle]
 * @param  a [point a]
 * @param  b [point b]
 * @param  c [point c]
 * @return   [the generated plane]
 */
ex_plane_t ex_triangle_to_plane(const vec3 a, const vec3 b, const vec3 c);

/**
 * [ex_signed_distance_to_plane]
 * @param  base_point [origin point]
 * @param  plane      [the plane]
 * @return            [the signed distance]
 */
float ex_signed_distance_to_plane(const vec3 base_point, const ex_plane_t *plane);

/**
 * [ex_is_front_facing]
 * @param  plane     [the plane to check]
 * @param  direction [the direction vector]
 * @return           [1 if front-facing]
 */
int ex_is_front_facing(ex_plane_t *plane, const vec3 direction);

/**
 * [ex_check_point_in_triangle]
 * @param  point [the point to check]
 * @param  p1    [tri point a]
 * @param  p2    [tri point b]
 * @param  p3    [tri point c]
 * @return       [1 if point resides in the triangle]
 */
int ex_check_point_in_triangle(const vec3 point, const vec3 p1, const vec3 p2, const vec3 p3);

/**
 * [ex_get_lowest_root]
 * @param  a    []
 * @param  b    []
 * @param  c    []
 * @param  max  [max root]
 * @param  root [pointer to the root]
 * @return      [1 if solution exists]
 */
int ex_get_lowest_root(float a, float b, float c, float max, float *root);

/**
 * [ray_in_tri Möller–Trumbore intersection algorithm]
 * @param  from      [starting point]
 * @param  to        [directional vector]
 * @param  v0        [tri p1]
 * @param  v1        [tri p2]
 * @param  v2        [tri p3]
 * @param  intersect [returned intersect point]
 * @return           [1 if intersection occurs, otherwise 0]
 */
int ray_in_tri(vec3 from, vec3 to, vec3 v0, vec3 v1, vec3 v2, vec3 intersect);

/**
 * [ex_collision_check_triangle the main collision iteration]
 * @param packet [the collision packet]
 * @param p1     [tri point a]
 * @param p2     [tri point b]
 * @param p3     [tri point c]
 */
void ex_collision_check_triangle(ex_coll_packet_t *packet, const vec3 p1, const vec3 p2, const vec3 p3);

#endif // EX_COLLISION_H