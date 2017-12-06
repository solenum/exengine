#include "entity.h"
#include "exe_list.h"
#include "model.h"
#include <stdlib.h>
#include <string.h>

#define VERY_CLOSE_DIST 0.005f
#define SLOPE_WALK_ANGLE 0.9f

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

void ex_entity_collide_and_slide(ex_entity_t *entity, vec3 gravity)
{
  memcpy(entity->packet.r3_position, entity->position, sizeof(vec3));
  memcpy(entity->packet.r3_velocity, entity->velocity, sizeof(vec3));
  memcpy(entity->packet.e_radius,    entity->radius,   sizeof(vec3));
  entity->packet.r3_velocity[1] = 0.0f;

  // lets get e-spacey?
  vec3 e_position, e_velocity, final_position;
  vec3_div(e_position, entity->packet.r3_position, entity->packet.e_radius);
  vec3_div(e_velocity, entity->packet.r3_velocity, entity->packet.e_radius);

  // base_point += e_velocity * very_close_dist
  // dumb fix lol
  vec3 temp;
  // vec3_scale(temp, entity->packet.e_velocity, 1.0f);
  // vec3_sub(e_position, e_position, temp);
  // vec3_add(e_velocity, e_velocity, temp);

  // do velocity iteration
  entity->packet.depth = 0;
  ex_entity_collide_with_world(entity, final_position, e_position, e_velocity);

  // do gravity iteration
  memcpy(entity->packet.r3_velocity, gravity, sizeof(vec3));
  vec3_div(e_velocity, gravity, entity->packet.e_radius);
  entity->packet.depth = 0;
  ex_entity_collide_with_world(entity, final_position, final_position, e_velocity);

  // finally set entity position & velocity
  vec3_mul(entity->position, final_position, entity->packet.e_radius);
  vec3_sub(entity->velocity, entity->position, entity->packet.r3_position);
}

void ex_entity_collide_with_world(ex_entity_t *entity, vec3 out_position, vec3 e_position, vec3 e_velocity)
{
  if (entity->packet.depth > 5)
    return;

  // check for collision
  vec3 temp;
  vec3_norm(temp, e_velocity);
  memcpy(entity->packet.e_norm_velocity, temp, sizeof(vec3));
  memcpy(entity->packet.e_velocity, e_velocity, sizeof(vec3));
  memcpy(entity->packet.e_base_point, e_position, sizeof(vec3));
  entity->packet.found_collision = 0;
  entity->packet.nearest_distance = FLT_MAX;

  ex_entity_check_collision(entity);

  // no collision move along
  if (entity->packet.found_collision == 0) {
    vec3_add(out_position, e_position, e_velocity);
    return;
  }

  // collision, panic!
  vec3 dest_point, new_base_point;
  vec3_add(dest_point, e_position, e_velocity);
  memcpy(new_base_point, e_position, sizeof(vec3));

  // only update if we are very close
  // or move very close
  if (entity->packet.nearest_distance >= VERY_CLOSE_DIST) {
    vec3 v;
    vec3_trim(v, e_velocity, entity->packet.nearest_distance - VERY_CLOSE_DIST);
    vec3_add(new_base_point, entity->packet.e_base_point, v);
    
    vec3_norm(v, v);

    vec3_scale(temp, v, VERY_CLOSE_DIST);
    vec3_sub(entity->packet.intersect_point, entity->packet.intersect_point, temp);
  }

  // determin sliding plane
  vec3 slide_plane_origin, slide_plane_normal;
  memcpy(slide_plane_origin, entity->packet.intersect_point, sizeof(vec3));
  vec3_sub(slide_plane_normal, new_base_point, entity->packet.intersect_point);
  vec3_norm(slide_plane_normal, slide_plane_normal);

  ex_plane_t sliding_plane = ex_plane_new(slide_plane_origin, slide_plane_normal);

  float slide_factor = ex_signed_distance_to_plane(dest_point, &sliding_plane);

  vec3 new_dest_point;
  vec3_scale(temp, slide_plane_normal, slide_factor);
  vec3_sub(new_dest_point, dest_point, temp);

  // new velocity for next iteration
  vec3 new_velocity;
  vec3_sub(new_velocity, new_dest_point, entity->packet.intersect_point);
  memcpy(out_position, new_base_point, sizeof(vec3));

  // dont recurse if velocity is tiny
  if (vec3_len(new_velocity) < VERY_CLOSE_DIST)
    return;

  entity->packet.depth++;

  // down the rabbit hole we go
  ex_entity_collide_with_world(entity, out_position, out_position, new_velocity);
}

void ex_entity_check_collision(ex_entity_t *entity)
{
  ex_rect_t r;
  vec3_sub(r.min, entity->position, entity->radius);
  vec3_add(r.max, entity->position, entity->radius);
  
  list_t *data = list_new();
  ex_octree_get_colliding(entity->scene->coll_tree, &r, data);

  vec3 *vertices = entity->scene->coll_vertices;
  while (data->data != NULL) {
    ex_octree_data_t *oct_data = data->data;
    uint32_t *indices = oct_data->data;
    for (int i=0; i<oct_data->len; i++) {
      vec3 a,b,c;
      vec3_div(a, vertices[indices[i]+0], entity->packet.e_radius);
      vec3_div(b, vertices[indices[i]+1], entity->packet.e_radius);
      vec3_div(c, vertices[indices[i]+2], entity->packet.e_radius);
      ex_collision_check_triangle(&entity->packet, a, b, c);
    }

    if (data->next != NULL)
      data = data->next;
    else
      break;
  }

  ex_octree_clean_colliding(data);
}

void ex_entity_check_grounded(ex_entity_t *entity, double dt)
{
  vec3 vel = {0.0f, -0.1f, 0.0f};
  memcpy(entity->packet.r3_position, entity->position, sizeof(vec3));
  memcpy(entity->packet.r3_velocity, vel, sizeof(vec3));
  memcpy(entity->packet.e_radius,    entity->radius,   sizeof(vec3));

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
      vec3_mul(entity->packet.intersect_point, entity->packet.intersect_point, entity->packet.e_radius);
      entity->position[1] = entity->packet.intersect_point[1] + entity->radius[1] + VERY_CLOSE_DIST;
      if (entity->velocity[1] > 0.0f)
        entity->velocity[1] = 0.0f;
    } else {
      entity->grounded = 0;
    }
  }
}

void ex_entity_update(ex_entity_t *entity, double dt)
{
  vec3 gravity = {0.0f, entity->velocity[1] * dt, 0.0f};
  vec3_scale(entity->velocity, entity->velocity, dt);
  ex_entity_collide_and_slide(entity, gravity);
  vec3_scale(entity->velocity, entity->velocity, 1.0f / dt);
  ex_entity_check_grounded(entity, dt);
}