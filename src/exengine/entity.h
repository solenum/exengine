#ifndef EX_ENTITY_H
#define EX_ENTITY_H

#include "mathlib.h"
#include "collision.h"
#include "octree.h"
#include "scene.h"

typedef struct {
  vec3 position, velocity, radius;
  coll_packet_t packet;
  scene_t *scene;
  int grounded;
} entity_t;

entity_t* entity_new(scene_t *scene, vec3 radius);

void entity_collide_and_slide(entity_t *entity, vec3 gravity);

void entity_collide_with_world(entity_t *entity, vec3 out_position, vec3 e_position, vec3 e_velocity);

void entity_check_collision(entity_t *entity);

void entity_check_grounded(entity_t *entity);

void entity_update(entity_t *entity, double dt);

#endif // EX_ENTITY_H