/* entity
  The 3d entity component of the engine,
  defined as an ellipsoid that slides around
  the game level.

  Part of the response step (as well as detection step)
  is based on the following paper by Kasper Fauerby.
  http://www.peroxide.dk/papers/collision/collision.pdf

  A more robust response step has been implemented
  based on the follow up paper to the above by 
  Jeff Linahan.
  https://arxiv.org/ftp/arxiv/papers/1211/1211.0059.pdf
*/

#ifndef EX_ENTITY_H
#define EX_ENTITY_H

#include "math/mathlib.h"
#include "math/collision.h"
#include "math/octree.h"
#include "render/scene.h"

typedef struct {
  vec3 position, velocity, radius;
  ex_coll_packet_t packet;
  ex_scene_t *scene;
  int grounded;
} ex_entity_t;

/**
 * [ex_entity_new defines a new entity]
 * @param  scene  [the scene the entity will reside in]
 * @param  radius [the radius of the ellipsoid]
 * @return        [the new entity]
 */
ex_entity_t* ex_entity_new(ex_scene_t *scene, vec3 radius);

/**
 * [ex_entity_collide_and_slide]
 * @param entity [entity to update]
 */
void ex_entity_collide_and_slide(ex_entity_t *entity);

/**
 * [ex_entity_collide_with_world]
 * @param entity     [entity to update]
 * @param e_position [initial position]
 * @param e_velocity [initial velocity]
 */
void ex_entity_collide_with_world(ex_entity_t *entity, vec3 e_position, vec3 e_velocity);

/**
 * [ex_entity_check_collision check collision against scene quadtree]
 * @param entity [entity to check against]
 */
void ex_entity_check_collision(ex_entity_t *entity);

/**
 * [ex_entity_check_grounded checks if entity is on a ground]
 * @param entity [entity to check]
 */
void ex_entity_check_grounded(ex_entity_t *entity);

/**
 * [ex_entity_update updates an entity, calling the above functions]
 * @param entity [entity to update]
 * @param dt     [delta time]
 */
void ex_entity_update(ex_entity_t *entity, double dt);

#endif // EX_ENTITY_H