#include "entity.h"
#include "exe_list.h"
#include "model.h"
#include <stdlib.h>
#include <string.h>

#define VERY_CLOSE_DIST 0.005f
#define SLOPE_WALK_ANGLE 0.98f

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
  vec3 gravity = {0.0f, entity->packet.r3_velocity[1], 0.0f};
  entity->packet.r3_velocity[1] = 0.0f;

  // lets get e-spacey?
  vec3 e_position, e_velocity, final_position;
  vec3_div(e_position, entity->packet.r3_position, entity->packet.e_radius);
  vec3_div(e_velocity, entity->packet.r3_velocity, entity->packet.e_radius);

  // do velocity iteration
  entity->packet.depth = 0;
  ex_entity_collide_with_world(entity, e_position, e_velocity);

  // do gravity iteration
  memcpy(entity->packet.r3_velocity, gravity, sizeof(vec3));
  vec3_div(e_velocity, gravity, entity->packet.e_radius);
  ex_entity_collide_with_world(entity, e_position, e_velocity);

  // finally set entity position & velocity
  vec3_mul(entity->position, e_position, entity->packet.e_radius);
  vec3_sub(entity->velocity, entity->position, entity->packet.r3_position);
}

void ex_entity_collide_with_world(ex_entity_t *entity, vec3 e_position, vec3 e_velocity)
{
  ex_plane_t first_plane;
  vec3 dest, src;
  vec3_add(dest, e_position, e_velocity);
  memcpy(src, e_position, sizeof(vec3));

  // check for collision
  vec3 temp;

  for (int i=0; i<3; ++i) {
    // setup coll packet
    vec3_norm(temp, e_velocity);
    memcpy(entity->packet.e_norm_velocity, temp, sizeof(vec3));
    memcpy(entity->packet.e_velocity, e_velocity, sizeof(vec3));
    memcpy(entity->packet.e_base_point, e_position, sizeof(vec3));
    memcpy(entity->packet.e_radius, entity->radius, sizeof(vec3));
    entity->packet.found_collision = 0;
    entity->packet.nearest_distance = FLT_MAX;
    
    // check for collision
    ex_entity_check_collision(entity);

    // no collision move along
    if (entity->packet.found_collision == 0) {
      memcpy(e_position, dest, sizeof(vec3));
      return;
    }

    // point touching tri
    vec3 touch_point;
    vec3_scale(temp, e_velocity, entity->packet.t);
    vec3_add(touch_point, e_position, temp);

    double dist = vec3_len(e_velocity) * entity->packet.t;
    double short_dist = MAX(dist - 1.0f, 0.0f);
    
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
      double long_radius = 1.0f + VERY_CLOSE_DIST;

      first_plane = ex_plane_new(slide_plane_origin, slide_plane_normal);

      double dist_to_plane  = ex_signed_distance_to_plane(dest, &first_plane) - long_radius;

      vec3_scale(temp, first_plane.normal, dist_to_plane);
      vec3_sub(dest, dest, temp);

      vec3_sub(e_velocity, dest, e_position);
    } else if (i == 1) {
      ex_plane_t second_plane = ex_plane_new(slide_plane_origin, slide_plane_normal);

      vec3 crease;
      vec3_mul_cross(crease, first_plane.normal, second_plane.normal);

      vec3_sub(temp, dest, e_position);
      double dis = vec3_mul_inner(temp, crease);
      vec3_norm(crease, crease);

      vec3_scale(e_velocity, crease, dis);
      vec3_add(dest, e_position, e_velocity);
    }
  }

  memcpy(e_position, dest, sizeof(vec3));
}

void ex_entity_check_collision(ex_entity_t *entity)
{
  ex_rect_t r;
  vec3_sub(r.min, entity->position, entity->radius);
  vec3_sub(r.min, r.min, entity->radius);
  vec3_add(r.max, entity->position, entity->radius);
  vec3_add(r.max, r.max, entity->radius);
 
  int count = 0;
  ex_octree_get_colliding_count(entity->scene->coll_tree, &r, &count);

  if (count <= 0)
    return;

  // printf("%i\n", count);

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

void ex_entity_check_grounded(ex_entity_t *entity, double dt)
{
  vec3 vel = {0.0f, -(entity->radius[1]+0.25f), 0.0f};
  memcpy(entity->packet.r3_position, entity->position, sizeof(vec3));
  memcpy(entity->packet.r3_velocity, vel, sizeof(vec3));
  memcpy(entity->packet.e_radius,    entity->radius,   sizeof(vec3));
  entity->packet.r3_position[1] += entity->radius[1];

  vec3 e_position, e_velocity;
  vec3_div(e_position, entity->packet.r3_position, entity->packet.e_radius);
  vec3_div(e_velocity, vel, entity->packet.e_radius);

  vec3_norm(entity->packet.e_norm_velocity, e_velocity);
  memcpy(entity->packet.e_norm_velocity, e_velocity, sizeof(vec3));
  memcpy(entity->packet.e_velocity, e_velocity, sizeof(vec3));
  memcpy(entity->packet.e_base_point, e_position, sizeof(vec3));
  entity->packet.found_collision = 0;
  entity->packet.nearest_distance = FLT_MAX;

  ex_entity_check_collision(entity);

  if (entity->packet.found_collision == 0) {
    entity->grounded = 0;
  } else {
    vec3 dest_point, new_base_point;
    vec3_add(dest_point, e_position, e_velocity);
    memcpy(new_base_point, e_position, sizeof(vec3));

    // get the slope angle
    vec3 slide_plane_normal;
    vec3_sub(slide_plane_normal, new_base_point, entity->packet.intersect_point);
    vec3_norm(slide_plane_normal, slide_plane_normal);
    float slope = vec3_mul_inner(slide_plane_normal, (vec3){0.0f, 1.0f, 0.0f});

    if (slope > SLOPE_WALK_ANGLE) {
      entity->grounded = 1;

      // snap to surface (also stepup)
      vec3_mul(entity->packet.intersect_point, entity->packet.intersect_point, entity->packet.e_radius);
      entity->position[1] = entity->packet.intersect_point[1] + entity->radius[1] + VERY_CLOSE_DIST;
    } else {
      entity->grounded = 0;
    }
  }
}

void ex_entity_update(ex_entity_t *entity, double dt)
{
  vec3_scale(entity->velocity, entity->velocity, dt);
  ex_entity_collide_and_slide(entity);
  vec3_scale(entity->velocity, entity->velocity, 1.0f / dt);
  ex_entity_check_grounded(entity, dt);
}