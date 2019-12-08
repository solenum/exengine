/* scene
  The scene handler, where the magic happens.

  This contains everything you
  want to render each frame, as
  well as your collision vertices
  which are stored in an internal
  octree.

  Currently it uses a deferred renderer,
  and has semi-function light culling.

  The following are features that need implementing:
  * Frustrum culling for models, lights etc
*/

#ifndef EX_SCENE_H
#define EX_SCENE_H

#include "render/render.h"
#include "util/list.h"
#include "math/octree.h"

#include "glad/glad.h"
#include <SDL2/SDL.h>

#define EX_MAX_POINT_LIGHTS 500
#define EX_MAX_SPOT_LIGHTS 128
#define EX_MAX_REFLECTIONS 50

// this should be the value of the biggest define above
#define EX_SCENE_BIGGEST_LIGHT 500

#define EX_SCENE_MAX_MODELS 500

/*
  The renderer feature flags,
  OR these together in the flags argument
  of ex_scene_new to enable the
  renderer features you want.
*/
#define EX_SCENE_SSAO 1
#define EX_SCENE_DEFERRED 2

typedef struct {
  vec3 gravity;
  
  ex_list_t *coll_list;
  ex_octree_t *coll_tree;
  int collision_built;
  vec3 *coll_vertices;
  size_t coll_vertices_last;

  ex_renderable_t renderables;
} ex_scene_t;

/**
 * [ex_scene_new defines a new scene]
 * @param  flags [the features to enable]
 * @return       [the new scene]
 */
ex_scene_t* ex_scene_new(uint8_t flags);

/**
 * [ex_scene_add_collision add a models vertices to the coll tree]
 * @param s [the scene to use]
 * @param m [the model which contains the vertices]
 *
 * This could do with being more generic
 * and being moved away from models.
 */
void ex_scene_add_collision(ex_scene_t *s, ex_model_t *m);

/**
 * [ex_scene_build_collision build the collision tree]
 * @param s [the scene to use]
 */
void ex_scene_build_collision(ex_scene_t *s);

/**
 * [ex_scene_add_model add a model to the render list]
 * @param s [the scene]
 * @param m [the model to add]
 */
void ex_scene_add_model(ex_scene_t *s, ex_model_t *m);

/**
 * [ex_scene_remove_model remove a model from the render list]
 * @param s [the scene]
 * @param m [the model to remove]
 */
void ex_scene_remove_model(ex_scene_t *s, ex_model_t *m);

/**
 * [ex_scene_add_pointlight]
 * @param s  [the scene to use]
 * @param pl [the pointlight to add]
 */
void ex_scene_add_pointlight(ex_scene_t *s, ex_point_light_t *pl);

/**
 * [ex_scene_update builds collision, updates models etc]
 * @param s          [the scene to use]
 * @param delta_time []
 */
void ex_scene_update(ex_scene_t *s, float delta_time);

/**
 * [ex_scene_draw render the scene]
 * @param s        [the scene to use]
 * @param x        [x offset]
 * @param y        [y offset]
 * @param width    [render width]
 * @param height   [render height]
 * @param matrices [the camera matrices]
 */
void ex_scene_draw(ex_scene_t *s, int x, int y, int width, int height, ex_camera_matrices_t *matrices);

/**
 * [ex_scene_resize resize internal frambuffers]
 * @param s      [the scene to resize]
 * @param width  [new width]
 * @param height [new height]
 */
void ex_scene_resize(ex_scene_t *s, int width, int height);

/**
 * [ex_scene_destroy cleanup scene data]
 * @param s [the scene to destroy]
 */
void ex_scene_destroy(ex_scene_t *s);

#endif // EX_SCENE_H