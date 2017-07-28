#ifndef SCENE_H
#define SCENE_H

#include "exe_list.h"
#include "texture.h"
#include "shader.h"
#include "skybox.h"
#include "camera.h"
#include "dbgui.h"
#include "pointlight.h"
#include "dirlight.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define MAX_POINT_LIGHTS 512

typedef struct {
  GLuint shader;
  list_t *model_list, *texture_list;
  fps_camera_t *fps_camera;
  skybox_t *skybox;
  vec3 gravity;
  point_light_t *point_lights[MAX_POINT_LIGHTS];
  dir_light_t *dir_light;
  
  /* dbug vars */
  int dynplightc, shdplightc, plightc, dlightc, modelc;
} scene_t;

scene_t* scene_new();

void scene_add_pointlight(scene_t *s, point_light_t *pl);

void scene_update(scene_t *s, float delta_time);

void scene_draw(scene_t *s);

void scene_manage_lights(scene_t *s);

void scene_dbgui(scene_t *s);

void scene_render_models(scene_t *s, GLuint shader, int shadows);

GLuint scene_add_texture(scene_t *s, const char *file);

void scene_destroy(scene_t *s);

#endif // SCENE_H