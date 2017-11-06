#include "collision.h"
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

// use signbit(a) instead?
#define in(a) *((uint32_t*) &a)

ex_plane_t ex_plane_new(const vec3 a, const vec3 b)
{
  ex_plane_t plane;
  memcpy(plane.origin, a, sizeof(vec3));
  memcpy(plane.normal, b, sizeof(vec3));
  plane.equation[0] = b[0];
  plane.equation[1] = b[1];
  plane.equation[2] = b[2];
  plane.equation[3] = -(b[0] * a[0] + b[1] * a[1] + b[2] * a[2]);

  return plane;
}

ex_plane_t ex_triangle_to_plane(const vec3 a, const vec3 b, const vec3 c)
{
  vec3 ba, ca;
  vec3_sub(ba, b, a);
  vec3_sub(ca, c, a);

  vec3 temp;
  vec3_mul_cross(temp, ba, ca);
  vec3_norm(temp, temp);

  ex_plane_t plane;
  memcpy(plane.origin, a, sizeof(vec3));
  memcpy(plane.normal, temp, sizeof(vec3));

  plane.equation[0] = temp[0];
  plane.equation[1] = temp[1];
  plane.equation[2] = temp[2];
  plane.equation[3] = -(temp[0] * a[0] + temp[1] * a[1] + temp[2] * a[2]);

  return plane;
}

double ex_signed_distance_to_plane(const vec3 base_point, const ex_plane_t *plane)
{
  return vec3_mul_inner(base_point, plane->normal) - vec3_mul_inner(plane->normal, plane->origin);// + plane->equation[3];
  // return vec3_mul_inner(base_point, plane->normal) + plane->equation[3];
}

int ex_is_front_facing(ex_plane_t *plane, const vec3 direction)
{
  double f = vec3_mul_inner(plane->normal, direction);
  
  if (f <= 0.0)
    return 1;

  return 0;
}

int ex_check_point_in_triangle(const vec3 point, const vec3 p1, const vec3 p2, const vec3 p3)
{
  /*
  vec3 e10, e20;
  vec3_sub(e10, p2, p1);
  vec3_sub(e20, p3, p1);
  float a = vec3_mul_inner(e10, e10);
  float b = vec3_mul_inner(e10, e20);
  float c = vec3_mul_inner(e20, e20);
  float ac_bb = (a * c) - (b * b);
  vec3 vp = {point[0] - p1[0], point[1] - p1[1], point[2] - p1[2]};
  float d = vec3_mul_inner(vp, e10);
  float e = vec3_mul_inner(vp, e20);
  float x = (d * c) - (e * b);
  float y = (e * a) - (d * d);
  float z = x + y - ac_bb;
  return (( in(z)& ~(in(x)|in(y)) ) & 0x80000000);*/

  vec3 u, v, w, vw, vu, uw, uv;
  vec3_sub(u, p2, p1);
  vec3_sub(v, p3, p1);
  vec3_sub(w, point, p1);
  
  vec3_mul_cross(vw, v, w);
  vec3_mul_cross(vu, v, u);

  if (vec3_mul_inner(vw, vu) < 0.0f) {
    return 0;
  }

  vec3_mul_cross(uw, u, w);
  vec3_mul_cross(uv, u, v);

  if (vec3_mul_inner(uw, uv) < 0.0f) {
    return 0;
  }

  float d = vec3_len(uv);
  float r = vec3_len(vw) / d;
  float t = vec3_len(uw) / d;

  return ((r + t) <= 1.0f);
}

int ex_get_lowest_root(float a, float b, float c, float max, float *root)
{
  // check if solution exists
  float determinant = b*b - 4.0f*a*c;

  // if negative there is no solution
  if (determinant < 0.0f)
    return 0;

  // calculate two roots
  float sqrtD = sqrtf(determinant);
  float r1 = (-b - sqrtD) / (2.0f*a);
  float r2 = (-b + sqrtD) / (2.0f*a);

  // set x1 <= x2
  if (r1 > r2) {
    float temp = r2;
    r2 = r1;
    r1 = temp;
  }

  // get lowest root
  if (r1 > 0 && r1 < max) {
    *root = r1;
    return 1;
  }

  if (r2 > 0 && r2 < max) {
    *root = r2;
    return 1;
  }

  // no solutions
  return 0;
}

void ex_collision_check_triangle(ex_coll_packet_t *packet, const vec3 p1, const vec3 p2, const vec3 p3)
{
  ex_plane_t plane = ex_triangle_to_plane(p1, p2, p3);

  // only check front facing triangles
  if (!ex_is_front_facing(&plane, packet->e_norm_velocity))
    return;
  
  // get interval of plane intersection
  double t0, t1;
  int embedded_in_plane = 0;

  // signed distance from sphere to point on plane
  double signed_dist_to_plane = ex_signed_distance_to_plane(packet->e_base_point, &plane);

  // cache this as we will reuse
  float normal_dot_vel = vec3_mul_inner(plane.normal, packet->e_velocity);

  // if sphere is moving parrallel to plane
  if (normal_dot_vel == 0.0f) {
    if (fabs(signed_dist_to_plane) >= 1.0f) {
      // no collision possible 
      return;
    } else {
      // sphere is in plane in whole range [0..1]
      embedded_in_plane = 1;
      t0 = 0.0;
      t1 = 1.0;
    }
  } else {
    // N dot D is not 0, calc intersect interval
    float nvi = 1.0f / normal_dot_vel;
    t0=(-1.0 - signed_dist_to_plane) * nvi;
    t1=( 1.0 - signed_dist_to_plane) * nvi;

    // swap so t0 < t1
    if (t0 > t1) {
      double temp = t1;
      t1 = t0;
      t0 = temp;
    }

    // check that at least one result is within range
    if (t0 > 1.0 || t1 < 0.0) {
      // both values outside range [0,1] so no collision
      return;
    }

    // clamp to [0,1]
    if (t0 < 0.0) t0 = 0.0;
    if (t1 < 0.0) t1 = 0.0;
    if (t0 > 1.0) t0 = 1.0;
    if (t1 > 1.0) t1 = 1.0;
  }

  // time to check for a collision
  vec3 collision_point;
  int found_collision = 0;
  float t = 1.0;

  // first check collision with the inside of the triangle
  if (embedded_in_plane == 0) {
    vec3 plane_intersect, temp;
    vec3_sub(plane_intersect, packet->e_base_point, plane.normal);
    vec3_scale(temp, packet->e_velocity, t0);
    vec3_add(plane_intersect, plane_intersect, temp);

    if (ex_check_point_in_triangle(plane_intersect, p1, p2, p3)) {
      found_collision = 1;
      t = t0;
      memcpy(collision_point, plane_intersect, sizeof(vec3));
    }
  }


  // no collision yet, check against points and edges
  if (found_collision == 0) {
    vec3 velocity, base, temp;
    memcpy(velocity, packet->e_velocity, sizeof(vec3));
    memcpy(base, packet->e_base_point, sizeof(vec3));
  
    float velocity_sqrt_length = vec3_len2(velocity);
    float a,b,c;
    float new_t;
  
    // equation is a*t^2 + b*t + c = 0
    // check against points
    a = velocity_sqrt_length;

    // p1
    vec3_sub(temp, base, p1);
    b = 2.0f*(vec3_mul_inner(velocity, temp));
    vec3_sub(temp, p1, base);
    c = vec3_len2(temp) - 1.0;
    if (ex_get_lowest_root(a, b, c, t, &new_t) == 1) {
      t = new_t;
      found_collision = 1;
      memcpy(collision_point, p1, sizeof(vec3));
    }

    // p2
    if (found_collision == 0) {
      vec3_sub(temp, base, p2);
      b = 2.0f*(vec3_mul_inner(velocity, temp));
      vec3_sub(temp, p2, base);
      c = vec3_len2(temp) - 1.0;
      if (ex_get_lowest_root(a, b, c, t, &new_t) == 1) {
        t = new_t;
        found_collision = 1;
        memcpy(collision_point, p2, sizeof(vec3));
      }
    }

    // p3
    if (found_collision == 0) {
      vec3_sub(temp, base, p3);
      b = 2.0f*(vec3_mul_inner(velocity, temp));
      vec3_sub(temp, p3, base);
      c = vec3_len2(temp) - 1.0;
      if (ex_get_lowest_root(a, b, c, t, &new_t) == 1) {
        t = new_t;
        found_collision = 1;
        memcpy(collision_point, p3, sizeof(vec3));
      }
    }

    // check against edges
    // p1 -> p2
    vec3 edge, base_to_vertex;
    vec3_sub(edge, p2, p1);
    vec3_sub(base_to_vertex, p1, base);
    float edge_sqrt_length        = vec3_len2(edge);
    float edge_dot_velocity       = vec3_mul_inner(edge, velocity);
    float edge_dot_base_to_vertex = vec3_mul_inner(edge, base_to_vertex);

    // calculate params for equation
    a = edge_sqrt_length * -velocity_sqrt_length + edge_dot_velocity * edge_dot_velocity;
    b = edge_sqrt_length * (2.0f * vec3_mul_inner(velocity, base_to_vertex)) -
        2.0f * edge_dot_velocity * edge_dot_base_to_vertex;
    c = edge_sqrt_length * (1.0f - vec3_len2(base_to_vertex)) +
        edge_dot_base_to_vertex * edge_dot_base_to_vertex;

    // do we collide against infinite edge
    if (ex_get_lowest_root(a, b, c, t, &new_t) == 1) {
      // check if intersect is within line segment
      float f = (edge_dot_velocity * new_t - edge_dot_base_to_vertex) / edge_sqrt_length;
      if (f >= 0.0f && f <= 1.0f) {
        t = new_t;
        found_collision = 1;
        vec3_scale(temp, edge, f);
        vec3_add(temp, p1, temp);
        memcpy(collision_point, temp, sizeof(vec3));
      }
    }


    // p2 -> p3
    vec3_sub(edge, p3, p2);
    vec3_sub(base_to_vertex, p2, base);
    edge_sqrt_length        = vec3_len2(edge);
    edge_dot_velocity       = vec3_mul_inner(edge, velocity);
    edge_dot_base_to_vertex = vec3_mul_inner(edge, base_to_vertex);

    // calculate params for equation
    a = edge_sqrt_length * -velocity_sqrt_length + edge_dot_velocity * edge_dot_velocity;
    b = edge_sqrt_length * (2.0f * vec3_mul_inner(velocity, base_to_vertex)) -
        2.0f * edge_dot_velocity * edge_dot_base_to_vertex;
    c = edge_sqrt_length * (1.0f - vec3_len2(base_to_vertex)) +
        edge_dot_base_to_vertex * edge_dot_base_to_vertex;

    // do we collide against infinite edge
    if (ex_get_lowest_root(a, b, c, t, &new_t) == 1) {
      // check if intersect is within line segment
      float f = (edge_dot_velocity * new_t - edge_dot_base_to_vertex) / edge_sqrt_length;
      if (f >= 0.0f && f <= 1.0f) {
        t = new_t;
        found_collision = 1;
        vec3_scale(temp, edge, f);
        vec3_add(temp, p2, temp);
        memcpy(collision_point, temp, sizeof(vec3));
      }
    }


    // p3 -> p1
    vec3_sub(edge, p1, p3);
    vec3_sub(base_to_vertex, p3, base);
    edge_sqrt_length        = vec3_len2(edge);
    edge_dot_velocity       = vec3_mul_inner(edge, velocity);
    edge_dot_base_to_vertex = vec3_mul_inner(edge, base_to_vertex);

    // calculate params for equation
    a = edge_sqrt_length * -velocity_sqrt_length + edge_dot_velocity * edge_dot_velocity;
    b = edge_sqrt_length * (2.0f * vec3_mul_inner(velocity, base_to_vertex)) -
        2.0f * edge_dot_velocity * edge_dot_base_to_vertex;
    c = edge_sqrt_length * (1.0f - vec3_len2(base_to_vertex)) +
        edge_dot_base_to_vertex * edge_dot_base_to_vertex;

    // do we collide against infinite edge
    if (ex_get_lowest_root(a, b, c, t, &new_t) == 1) {
      // check if intersect is within line segment
      float f = (edge_dot_velocity * new_t - edge_dot_base_to_vertex) / edge_sqrt_length;
      if (f >= 0.0f && f <= 1.0f) {
        t = new_t;
        found_collision = 1;
        vec3_scale(temp, edge, f);
        vec3_add(temp, p3, temp);
        memcpy(collision_point, temp, sizeof(vec3));
      }
    }
  }

  // set results
  if (found_collision == 1) {
    // distance to collision, t is time of collision
    float dist_to_coll = t*vec3_len(packet->e_velocity);
    
    // are we the closest hit?
    if (packet->found_collision == 0 || dist_to_coll < packet->nearest_distance) {
      packet->nearest_distance = dist_to_coll;
      memcpy(packet->intersect_point, collision_point, sizeof(vec3));
      packet->found_collision = 1;
    }
  }
}