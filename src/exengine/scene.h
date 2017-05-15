#ifndef SCENE_H
#define SCENE_H

#include "exe_list.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct {
  GLuint shader;
  list_t *model_list, *texture_list;
  list_t *point_light_list, *dir_light_list;
  fps_camera_t *fps_camera;
} scene_t;

scene_t* scene_new();

void scene_update(scene_t *s, float delta_time);

void scene_draw(scene_t *s);

void scene_render_models(scene_t *s, GLuint shader);

GLuint scene_add_texture(scene_t *s, const char *file);

void scene_destroy(scene_t *s);

#endif // SCENE_H