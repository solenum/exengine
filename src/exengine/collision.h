#ifndef EX_COLLISION_H
#define EX_COLLISION_H

#include "mathlib.h"

typedef struct {
  // r3 space
  vec3 r3_velocity, r3_position;

  // ellipsoid space
  vec3 e_radius;
  vec3 e_velocity;
  vec3 e_norm_velocity;
  vec3 e_base_point;

  // hit information
  int found_collision;
  float nearest_distance;
  vec3 intersect_point;

  // iteration depth
  int depth;
} ex_coll_packet_t;

typedef struct {
  vec3 origin;
  vec3 normal;
  float equation[4];
} ex_plane_t;

ex_plane_t ex_plane_new(const vec3 a, const vec3 b);

ex_plane_t ex_triangle_to_plane(const vec3 a, const vec3 b, const vec3 c);

double ex_signed_distance_to_plane(const vec3 base_point, const ex_plane_t *plane);

int ex_is_front_facing(ex_plane_t *plane, const vec3 direction);

int ex_check_point_in_triangle(const vec3 point, const vec3 p1, const vec3 p2, const vec3 p3);

int ex_get_lowest_root(float a, float b, float c, float max, float *root);

void ex_collision_check_triangle(ex_coll_packet_t *packet, const vec3 p1, const vec3 p2, const vec3 p3);

#endif // EX_COLLISION_H