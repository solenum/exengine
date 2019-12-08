#include <stdlib.h>
#include <string.h>
#include "util/entity.h"
#include "util/list.h"
#include "render/model.h"

#define SLIDE_BIAS 0.008
#define VERY_CLOSE_DIST 0.001
#define SLOPE_WALK_ANGLE 0.80
#define DOWN_DIRECTION -1.0
#define DOWN_AXIS 1 // y

ex_entity_t* ex_entity_new(ex_scene_t *scene, vec3 radius)
{
  ex_entity_t *e = malloc(sizeof(ex_entity_t));
  memcpy(e->radius,   radius, sizeof(vec3));
  memset(e->position, 0,      sizeof(vec3));
  memset(e->velocity, 0,      sizeof(vec3));
  e->scene = scene;
  e->grounded = 0;
  return e;
}

void ex_entity_collide_and_slide(ex_entity_t *entity)
{
  memcpy(entity->packet.r3_position, entity->position, sizeof(vec3));
  memcpy(entity->packet.r3_velocity, entity->velocity, sizeof(vec3));
  memcpy(entity->packet.e_radius,    entity->radius,   sizeof(vec3));

  // y velocity in a seperate pass
  vec3 gravity = {0.0f};
  gravity[DOWN_AXIS] = entity->packet.r3_velocity[DOWN_AXIS];
  entity->packet.r3_velocity[DOWN_AXIS] = 0.0f;

  // lets get e-spacey?
  vec3 e_position, e_velocity, final_position;
  vec3_div(e_position, entity->packet.r3_position, entity->packet.e_radius);
  vec3_div(e_velocity, entity->packet.r3_velocity, entity->packet.e_radius);

  // do velocity iteration
  ex_entity_collide_with_world(entity, e_position, e_velocity);

  // do gravity iteration
  memcpy(entity->packet.r3_velocity, gravity, sizeof(vec3));
  vec3_div(e_velocity, gravity, entity->packet.e_radius);
  ex_entity_collide_with_world(entity, e_position, e_velocity);

  // finally set entity position & velocity
  vec3_mul(entity->position, e_position, entity->packet.e_radius);
}

void ex_entity_collide_with_world(ex_entity_t *entity, vec3 e_position, vec3 e_velocity)
{
  ex_plane_t first_plane;
  vec3 dest, src;
  vec3_add(dest, e_position, e_velocity);
  memcpy(src, e_position, sizeof(vec3));

  // check for collision
  vec3 temp;

  for (int i=0; i<3; i++) {
    // setup coll packet
    vec3_norm(temp, e_velocity);
    memcpy(entity->packet.e_norm_velocity, temp, sizeof(vec3));
    memcpy(entity->packet.e_velocity, e_velocity, sizeof(vec3));
    memcpy(entity->packet.e_base_point, e_position, sizeof(vec3));
    memcpy(entity->packet.e_radius, entity->radius, sizeof(vec3));
    entity->packet.found_collision = 0;
    entity->packet.nearest_distance = FLT_MAX;
    entity->packet.t = 0.0f;
    
    // check for collision
    ex_entity_check_collision(entity);

    ex_entity_check_grounded(entity);

    // no collision move along
    if (entity->packet.found_collision == 0) {
      memcpy(e_position, dest, sizeof(vec3));
      return;
    }

    // point touching tri
    vec3 touch_point;
    vec3_scale(temp, e_velocity, entity->packet.t);
    vec3_add(touch_point, e_position, temp);

    float dist = vec3_len(e_velocity) * entity->packet.t;
    float short_dist = MAX(dist - 1.0f, 0.0f);
    
    vec3_norm(temp, e_velocity);
    vec3_scale(temp, temp, short_dist);
    vec3_add(e_position, e_position, temp);

    // calculate sliding plane
    vec3 slide_plane_origin, slide_plane_normal;
    
    // use intersect point as origin
    memcpy(slide_plane_origin, entity->packet.intersect_point, sizeof(vec3));
    
    // normal = touch_point - intersect_point
    // dont use normal from packet.plane.normal!
    vec3_sub(slide_plane_normal, touch_point, entity->packet.intersect_point);
    vec3_norm(slide_plane_normal, slide_plane_normal);

    if (i == 0) {
      float long_radius = 1.0 + VERY_CLOSE_DIST;

      first_plane = ex_plane_new(slide_plane_origin, slide_plane_normal);

      float dist_to_plane  = ex_signed_distance_to_plane(dest, &first_plane) - long_radius;

      vec3_scale(temp, first_plane.normal, dist_to_plane);
      vec3_sub(dest, dest, temp);

      vec3_sub(e_velocity, dest, e_position);
    } else if (i == 1) {
      ex_plane_t second_plane = ex_plane_new(slide_plane_origin, slide_plane_normal);

      vec3 crease;
      vec3_mul_cross(crease, first_plane.normal, second_plane.normal);

      vec3_sub(temp, dest, e_position);
      float dis = vec3_mul_inner(temp, crease);
      vec3_norm(crease, crease);

      vec3_scale(e_velocity, crease, dis);
      vec3_add(dest, e_position, e_velocity);
    }
  }

  memcpy(e_position, dest, sizeof(vec3));
}

void ex_entity_check_collision(ex_entity_t *entity)
{
  rect_t r;
  vec3_sub(r.min, entity->position, entity->radius);
  vec3_sub(r.min, r.min, entity->radius);
  vec3_add(r.max, entity->position, entity->radius);
  vec3_add(r.max, r.max, entity->radius);
 
  int count = 0;
  ex_octree_get_colliding_count(entity->scene->coll_tree, &r, &count);

  if (count <= 0)
    return;

  ex_octree_data_t *data = malloc(sizeof(ex_octree_data_t) * count);
  for (int i=0; i<count; i++) {
    data[i].data = NULL;
    data[i].len  = 0;
  }

  int index = 0;
  ex_octree_get_colliding(entity->scene->coll_tree, &r, data, &index);

  vec3 *vertices = entity->scene->coll_vertices;
  for (int i=0; i<count; i++) {
    uint32_t *indices = (uint32_t*)data[i].data;

    if (indices == NULL)
      continue;

    for (int k=0; k<data[i].len; k++) {
      vec3 a,b,c;
      vec3_div(a, vertices[indices[k]+0], entity->packet.e_radius);
      vec3_div(b, vertices[indices[k]+1], entity->packet.e_radius);
      vec3_div(c, vertices[indices[k]+2], entity->packet.e_radius);
      ex_collision_check_triangle(&entity->packet, a, b, c);
    }
  }

  free(data);
}

void ex_entity_check_grounded(ex_entity_t *entity)
{
  if (!entity->packet.found_collision)
    return;

  vec3 axis = {0.0f};
  axis[DOWN_AXIS] = 1.0f;

  vec3 a, b, c;
  vec3_mul(a, entity->packet.a, entity->radius);
  vec3_mul(b, entity->packet.b, entity->radius);
  vec3_mul(c, entity->packet.c, entity->radius);
  ex_plane_t plane = ex_triangle_to_plane(a, b, c);
  float f = vec3_mul_inner(plane.normal, axis);

  if (f >= SLOPE_WALK_ANGLE)
    entity->grounded = 1;
}

void ex_entity_update(ex_entity_t *entity, double dt)
{
  vec3 xz = {0.0f};
  xz[0] = entity->velocity[0];
  xz[2] = entity->velocity[2];
  
  // prevent sliding while standing still on a slope
  // change < 0.0f to > 0.0f if inverting y axis
  if (entity->grounded && fabs(vec2_len(xz)) < 0.1f && entity->velocity[1] < 0.0f)
    entity->velocity[1] = 0.0f;
  else
    entity->grounded = 0;
  
  dt = dt / 5.0;
  
  vec3_scale(entity->velocity, entity->velocity, dt);
  for (int i=0; i<5; i++)
    ex_entity_collide_and_slide(entity);
  vec3_sub(entity->velocity, entity->position, entity->packet.r3_position);
  vec3_scale(entity->velocity, entity->velocity, 1.0 / dt);
}

float raycast(ex_entity_t *entity, vec3 from, vec3 to, ex_plane_t *plane)
{
  vec3 a,b;
  memcpy(a, from, sizeof(vec3));
  vec3_add(b, from, to);

  rect_t r;
  vec3_min(r.min, a, b);
  vec3_max(r.max, a, b);
  
  int count = 0;
  ex_octree_get_colliding_count(entity->scene->coll_tree, &r, &count);

  if (count <= 0)
    return 0.0f;

  ex_octree_data_t *data = malloc(sizeof(ex_octree_data_t) * count);
  for (int i=0; i<count; i++) {
    data[i].data = NULL; 
    data[i].len  = 0;
  }

  int index = 0;
  ex_octree_get_colliding(entity->scene->coll_tree, &r, data, &index);

  size_t tri;
  vec3 *vertices = entity->scene->coll_vertices;
  float dist = FLT_MAX;
  vec3 intersect, nearest;
  for (int i=0; i<count; i++) {
    uint32_t *indices = (uint32_t*)data[i].data;

    if (indices == NULL)
      continue;

    for (int k=0; k<data[i].len; k++) {
      if (ray_in_tri(from, to, vertices[indices[k]+0], vertices[indices[k]+1], vertices[indices[k]+2], intersect)) {

        vec3 len;
        vec3_sub(len, from, intersect);
        float d = vec3_len(len);
        if (d < dist) {
          memcpy(nearest, intersect, sizeof(vec3));
          dist = d;
          tri = indices[k];
        }
      }
    }
  }

  if (dist < FLT_MAX && dist <= vec3_len(to)) {
    ex_plane_t p = ex_triangle_to_plane(vertices[tri], vertices[tri+1], vertices[tri+2]);
    memcpy(plane, &p, sizeof(ex_plane_t));
    
    free(data);
    return dist;
  }

  free(data);
  return 0;
}