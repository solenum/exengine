#ifndef COLLISION_H
#define COLLISION_H

#include "mathlib.h"

#define UNITS_PER_METER 100.0f

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
} coll_packet_t;

typedef struct {
  vec3 origin;
  vec3 normal;
  float equation[4];
} plane_t;

plane_t plane_new(const vec3 a, const vec3 b);

plane_t triangle_to_plane(const vec3 a, const vec3 b, const vec3 c);

float signed_distance_to_plane(const vec3 base_point, const plane_t *plane);

int is_front_facing(plane_t *plane, const vec3 direction);

int check_point_in_triangle(const vec3 point, const vec3 p1, const vec3 p2, const vec3 p3);

int get_lowest_root(float a, float b, float c, float max, float *root);

void collision_check_triangle(coll_packet_t *packet, const vec3 p1, const vec3 p2, const vec3 p3);

#endif // COLLISION_H