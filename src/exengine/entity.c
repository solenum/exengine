  #include "entity.h"
#include "exe_list.h"
#include "model.h"
#include <stdlib.h>
#include <string.h>

entity_t* entity_new(scene_t *scene, vec3 radius)
{
  entity_t *e = malloc(sizeof(entity_t));
  memcpy(e->radius,   radius, sizeof(vec3));
  memset(e->position, 0,      sizeof(vec3));
  memset(e->velocity, 0,      sizeof(vec3));
  e->scene = scene;
}

void entity_collide_and_slide(entity_t *entity, vec3 gravity)
{
  memcpy(entity->packet.r3_position, entity->position, sizeof(vec3));
  memcpy(entity->packet.r3_velocity, entity->velocity, sizeof(vec3));
  memcpy(entity->packet.e_radius,    entity->radius,   sizeof(vec3));

  // lets get e-spacey?
  vec3 e_position, e_velocity, final_position;
  vec3_div(e_position, entity->packet.r3_position, entity->packet.e_radius);
  vec3_div(e_velocity, entity->packet.r3_velocity, entity->packet.e_radius);

  // do velocity iteration
  entity->packet.depth = 0;
  entity_collide_with_world(entity, final_position, e_position, e_velocity);

  // convert back to r3 space
  vec3_mul(entity->packet.r3_position, final_position, entity->packet.e_radius);
  memcpy(entity->packet.r3_velocity, gravity, sizeof(vec3));

  // convert velocity to e-space
  vec3_div(e_velocity, gravity, entity->packet.e_radius);

  // do gravity iteration
  entity->packet.depth = 0;
  entity_collide_with_world(entity, final_position, final_position, e_velocity);

  // convert back to r3 space

  // finally set entity position
  vec3_mul(entity->position, final_position, entity->packet.e_radius);
  memset(entity->velocity, 0, sizeof(vec3));
}

void entity_collide_with_world(entity_t *entity, vec3 out_position, vec3 e_position, vec3 e_velocity)
{
  float unit_scale = UNITS_PER_METER / 100.0f;
  float very_close_dist = 0.005f * unit_scale;

  if (entity->packet.depth > 5)
    return;

  // check for collision
  vec3 temp;
  vec3_norm(temp, e_velocity);
  memcpy(entity->packet.e_norm_velocity, temp, sizeof(vec3));
  memcpy(entity->packet.e_velocity, e_velocity, sizeof(vec3));
  memcpy(entity->packet.e_base_point, e_position, sizeof(vec3));
  entity->packet.found_collision = 0;
  entity->packet.nearest_distance = HUGE_VAL;

  entity_check_collision(entity);

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
  if (entity->packet.nearest_distance >= very_close_dist) {
    vec3 v;
    vec3_trim(v, e_velocity, entity->packet.nearest_distance - very_close_dist);
    vec3_add(new_base_point, entity->packet.e_base_point, v);
    
    vec3_norm(v, v);

    vec3_scale(temp, v, very_close_dist);
    vec3_sub(entity->packet.intersect_point, entity->packet.intersect_point, temp);
  }

  // determin sliding plane
  vec3 slide_plane_origin, slide_plane_normal;
  memcpy(slide_plane_origin, entity->packet.intersect_point, sizeof(vec3));
  vec3_sub(slide_plane_normal, new_base_point, entity->packet.intersect_point);
  vec3_norm(slide_plane_normal, slide_plane_normal);

  plane_t sliding_plane = plane_new(slide_plane_origin, slide_plane_normal);

  double slide_factor = signed_distance_to_plane(dest_point, &sliding_plane);
  vec3 new_dest_point;
  vec3_scale(temp, slide_plane_normal, slide_factor);
  vec3_sub(new_dest_point, dest_point, temp);

  // new velocity for next iteration
  vec3 new_velocity;
  vec3_sub(new_velocity, new_dest_point, entity->packet.intersect_point);
  memcpy(out_position, new_base_point, sizeof(vec3));

  // dont recurse if velocity is tiny
  if (vec3_len(new_velocity) < very_close_dist)
    return;

  entity->packet.depth++;

  // down the rabbit hole we go
  entity_collide_with_world(entity, out_position, out_position, new_velocity);
}

void entity_check_collision(entity_t *entity)
{
  // check collision against triangles
  // **!CHEAP TESTING METHOD PLS REPLACE WITH OCTREE!** //
  list_node_t *n = entity->scene->model_list;
  while (n->data != NULL) {
    model_t *m = n->data;
    if (m->vertices != NULL) {
      vertex_t *v = m->vertices;
      for (int i=0; i<m->num_vertices; i) {
        vec3 a,b,c;
        vec3_div(a, v[i++].position, entity->packet.e_radius);
        vec3_div(b, v[i++].position, entity->packet.e_radius);
        vec3_div(c, v[i++].position, entity->packet.e_radius);
        collision_check_triangle(&entity->packet, a, b, c);
      }
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
  // **!CHEAP TESTING METHOD PLS REPLACE WITH OCTREE!** //
}

void entity_update(entity_t *entity)
{
  entity_collide_and_slide(entity, entity->scene->gravity);
}