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

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define EX_MAX_POINT_LIGHTS 500
#define EX_MAX_SPOT_LIGHTS 128
#define EX_MAX_REFLECTIONS 50

// this should be the value of the biggest define above
#define EX_SCENE_BIGGEST_LIGHT 500

typedef struct {
  GLuint shader, primshader;
  list_t *model_list, *texture_list, *coll_list;
  ex_fps_camera_t *fps_camera;
  ex_skybox_t *skybox;
  vec3 gravity;
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
} ex_scene_t;

ex_scene_t* scene_new();

void ex_scene_add_collision(ex_scene_t *s, ex_model_t *m);

void ex_scene_build_collision(ex_scene_t *s);

void ex_scene_add_pointlight(ex_scene_t *s, ex_point_light_t *pl);

void ex_scene_add_spotlight(ex_scene_t *s, ex_spot_light_t *pl);

void ex_scene_add_reflection(ex_scene_t *s, ex_reflection_t *r);

void ex_scene_update(ex_scene_t *s, float delta_time);

void ex_scene_draw(ex_scene_t *s, int x, int y, int width, int height);

void ex_scene_manage_lights(ex_scene_t *s);

void ex_scene_dbgui(ex_scene_t *s);

void ex_scene_render_models(ex_scene_t *s, GLuint shader, int shadows);

GLuint ex_scene_add_texture(ex_scene_t *s, const char *file);

void ex_scene_destroy(ex_scene_t *s);

#endif // EX_SCENE_H