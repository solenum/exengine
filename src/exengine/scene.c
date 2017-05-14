#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "model.h"
#include "pointlight.h"
#include "framebuffer.h"

scene_t* scene_new()
{
  // load shaders
  scene_t *s = malloc(sizeof(scene_t));
  s->shader = shader_compile("data/shader.vs", "data/shader.fs", NULL);

  // init lists
  s->model_list       = list_new();
  s->point_light_list = list_new();
  s->texture_list     = list_new();

  s->fps_camera = NULL;

  // init framebuffers etc
  framebuffer_init();

  // init point lights
  point_light_init();

  return s;
}

void scene_update(scene_t *s, float delta_time)
{
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    model_update(n->data, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

void scene_draw(scene_t *s)
{
  // render pointlight depth maps
  glCullFace(GL_BACK);
  list_node_t *n = s->point_light_list;
  while (n->data != NULL) {
    point_light_t *l = n->data;

    if (l->dynamic || l->update) {
      point_light_begin(l);
      scene_render_models(s, l->shader);
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // first rendering pass
  framebuffer_first();

  // render everything
  glUseProgram(s->shader);

  // update camera
  if (s->fps_camera != NULL)
    fps_camera_update(s->fps_camera, s->shader);

  // render ambient pass
  glUniform1i(glGetUniformLocation(s->shader, "u_ambient_pass"), 1);
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);
  scene_render_models(s, s->shader);

  // render point lights
  glEnable(GL_BLEND);
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glUniform1i(glGetUniformLocation(s->shader, "u_ambient_pass"), 0);
  n = s->point_light_list;
  while (n->data != NULL) {
    point_light_t *l = n->data;
    point_light_draw(l, s->shader);

    // render models
    scene_render_models(s, s->shader);
    
    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
  glDisable(GL_BLEND);


  // render screen quad
  framebuffer_render_quad();
}

void scene_render_models(scene_t *s, GLuint shader)
{
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    model_draw(n->data, shader);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

GLuint scene_add_texture(scene_t *s, const char *file)
{  
  // check if texture already exists
  list_node_t *n = s->texture_list;
  while (n->data != NULL) {
    texture_t *t = n->data;

    // compare file names
    if (strcmp(file, t->name) == 0) {
      // yep, return that one
      return t->id;
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // doesnt exist, create texture
  texture_t *t = texture_load(file);
  if (t != NULL) {
    // store it in the list
    list_add(s->texture_list, (void*)t);
    return t->id;
  }

  return 0;
}

void scene_destroy(scene_t *s)
{
  printf("Cleaning up scene\n");

  // cleanup models
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    model_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free model list
  list_destroy(s->model_list);

  // cleanup point lights
  n = s->point_light_list;
  while (n->data != NULL) {
    point_light_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free point light list
  list_destroy(s->point_light_list);

  // cleanup textures
  n = s->texture_list;
  while (n->data != NULL) {
    texture_t *t = n->data;
    
    // free texture data
    glDeleteTextures(1, &t->id);
    free(t);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free texture list
  list_destroy(s->texture_list);

  // cleanup cameras
  if (s->fps_camera != NULL)
    free(s->fps_camera);

  // cleanup framebuffers
  framebuffer_destroy();
}