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

#include "exe_list.h"
#include "texture.h"
#include "shader.h"
#include "skybox.h"
#include "camera.h"
#include "pointlight.h"
#include "dirlight.h"
#include "spotlight.h"
#include "octree.h"
#include "reflectionprobe.h"
#include "framebuffer.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
  GLuint shader, primshader, forwardshader, defaultshader;
  list_t *coll_list;
  ex_skybox_t *skybox;
  vec3 gravity;
  ex_model_t *models[EX_SCENE_MAX_MODELS];
  ex_point_light_t *point_lights[EX_MAX_POINT_LIGHTS];
  ex_spot_light_t *spot_lights[EX_MAX_SPOT_LIGHTS];
  ex_dir_light_t *dir_light;
  ex_reflection_t *reflection_probes[EX_MAX_REFLECTIONS];
  
  ex_octree_t *coll_tree;
  int collision_built;
  vec3 *coll_vertices;
  size_t coll_vertices_last;

  /* dbug vars */
  int dynplightc, shdplightc, plightc, dlightc, slightc, modelc;
  
  ex_framebuffer_t *framebuffer;

  /* rendering features */
  int ssao;
  int deferred;
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
 * [ex_scene_add_spotlight]
 * @param s  [the scene to use]
 * @param pl [the spotlight to add]
 */
void ex_scene_add_spotlight(ex_scene_t *s, ex_spot_light_t *pl);

/**
 * [ex_scene_add_reflection]
 * @param s [the scene to use]
 * @param r [the relfection probe to add]
 */
void ex_scene_add_reflection(ex_scene_t *s, ex_reflection_t *r);

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
 * [ex_scene_render_deferred]
 * @param s        [the scene to use]
 * @param x        [x offset]
 * @param y        [y offset]
 * @param width    [render width]
 * @param height   [render height]
 * @param matrices [the camera matrices]
 */
void ex_scene_render_deferred(ex_scene_t *s, int x, int y, int width, int height, ex_camera_matrices_t *matrices);

/**
 * [ex_scene_render_forward]
 * @param s        [the scene to use]
 * @param x        [x offset]
 * @param y        [y offset]
 * @param width    [render width]
 * @param height   [render height]
 * @param matrices [the camera matrices]
 */
void ex_scene_render_forward(ex_scene_t *s, int x, int y, int width, int height, ex_camera_matrices_t *matrices);

/**
 * [ex_scene_manage_lights cull lights]
 * @param s [the scene to use]
 *
 * Currently broken-ish, isn't very good
 * anyway.  Needs to be replaced with
 * generic frustrum culling.
 */
void ex_scene_manage_lights(ex_scene_t *s);

/**
 * [ex_scene_dbgui renders debug gui]
 * @param s [the scene to use]
 */
void ex_scene_dbgui(ex_scene_t *s);

/**
 * [ex_scene_render_models renders all scene models]
 * @param s       [the scene to use]
 * @param shader  [the shader to use]
 * @param shadows [1 if rendering shadow-casting models only]
 */
void ex_scene_render_models(ex_scene_t *s, GLuint shader, int shadows);

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