#ifndef EX_ENTITY_H
#define EX_ENTITY_H

#include "mathlib.h"
#include "collision.h"
#include "octree.h"
#include "scene.h"

typedef struct {
  vec3 position, velocity, radius;
  ex_coll_packet_t packet;
  ex_scene_t *scene;
  int grounded;
} ex_entity_t;

ex_entity_t* ex_entity_new(ex_scene_t *scene, vec3 radius);

void ex_entity_collide_and_slide(ex_entity_t *entity);

void ex_entity_collide_with_world(ex_entity_t *entity, vec3 e_position, vec3 e_velocity);

void ex_entity_check_collision(ex_entity_t *entity);

void ex_entity_check_grounded(ex_entity_t *entity, double dt);

void ex_entity_update(ex_entity_t *entity, double dt);

#endif // EX_ENTITY_H