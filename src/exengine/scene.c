#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "model.h"
#include "pointlight.h"
#include "dirlight.h"
#include "framebuffer.h"
#include "gbuffer.h"
#include "window.h"
#include "dbgui.h"

scene_t* scene_new()
{
  // load shaders
  scene_t *s = malloc(sizeof(scene_t));
  s->shader = shader_compile("data/shader.vs", "data/shader.fs", NULL);

  // init lists
  s->model_list       = list_new();
  s->point_light_list = list_new();
  s->dir_light_list   = list_new();
  s->texture_list     = list_new();

  s->fps_camera = NULL;

  // init framebuffers etc
  framebuffer_init();
  gbuffer_init();

  // init lights
  point_light_init();
  dir_light_init();

  // init skybox
  s->skybox = NULL;
  skybox_init();

  // init physics shiz
  memset(s->gravity, 0, sizeof(vec3));

  // init debug gui
  ex_dbgui_init();

  return s;
}

void scene_update(scene_t *s, float delta_time)
{
  ex_dbgprofiler.begin[ex_dbgprofiler_update] = (float)glfwGetTime();

  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    model_update(n->data, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  ex_dbgprofiler.end[ex_dbgprofiler_update] = (float)glfwGetTime();
}

void scene_draw(scene_t *s)
{
  // begin profiler
  ex_dbgprofiler.end[ex_dbgprofiler_other] = (float)glfwGetTime();
  ex_dbgui_end_profiler();
  ex_dbgui_begin_profiler();

  // render pointlight depth maps
  glCullFace(GL_BACK);
  ex_dbgprofiler.begin[ex_dbgprofiler_lighting_depth] = (float)glfwGetTime();
  list_node_t *n = s->point_light_list;
  while (n->data != NULL) {
    point_light_t *l = n->data;

    if ((l->dynamic || l->update) && l->is_shadow) {
      point_light_begin(l);
      scene_render_models(s, l->shader, 1);
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // render dirlight depth maps
  glCullFace(GL_BACK);
  n = s->dir_light_list;
  while (n->data != NULL) {
    dir_light_t *l = n->data;

    if (l->dynamic || l->update) {

      if (s->fps_camera != NULL) {
        memcpy(l->cposition, s->fps_camera->position, sizeof(vec3));
        l->cposition[1] = 0.0f;
      }

      dir_light_begin(l);

      scene_render_models(s, l->shader, 1);
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
  ex_dbgprofiler.end[ex_dbgprofiler_lighting_depth] = (float)glfwGetTime();

  fps_camera_update(s->fps_camera, gshader);

  // first geometry render pass
  gbuffer_first();
  glUseProgram(gshader);
  
  // debug poooo
  if (keys_down[GLFW_KEY_E])
    glUniform1i(glGetUniformLocation(gshader, "u_dont_norm"), 0);
  if (keys_down[GLFW_KEY_R])
    glUniform1i(glGetUniformLocation(gshader, "u_dont_norm"), 1);

  fps_camera_draw(s->fps_camera, gshader);
  scene_render_models(s, gshader, 0);

  // render stuffs
  framebuffer_first();
  glUseProgram(fbo_shader);

  int index = 0;
  list_node_t *pl_list = s->point_light_list;
  while (pl_list != NULL) {
    if (pl_list != NULL && pl_list->data != NULL) {
      point_light_t *pl = pl_list->data;
      point_light_draw(pl, fbo_shader, index);
    }

    if (pl_list != NULL && pl_list->next != NULL)
      pl_list = pl_list->next;
    else
      pl_list = NULL;
    
    glUniform1i(glGetUniformLocation(fbo_shader, "u_pl_count"), index);

    index++;
  }

  gbuffer_render(fbo_shader);
  fps_camera_draw(s->fps_camera, fbo_shader);
  framebuffer_render_quad();

  /*
  // render skybox
  if (s->skybox != NULL) {
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(s->skybox->shader);

    if (s->fps_camera != NULL)
      fps_camera_draw(s->fps_camera, s->skybox->shader);
    
    skybox_draw(s->skybox);
  }

  // render scene
  glUseProgram(s->shader);

  // debug poooo
  if (keys_down[GLFW_KEY_E])
    glUniform1i(glGetUniformLocation(s->shader, "u_dont_norm"), 0);
  if (keys_down[GLFW_KEY_R])
    glUniform1i(glGetUniformLocation(s->shader, "u_dont_norm"), 1);

  // render lit scene
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);

  list_node_t *pl_list = s->point_light_list;
  list_node_t *dl_list = s->dir_light_list;
  int ambient_pass = 1;
  ex_dbgprofiler.begin[ex_dbgprofiler_lighting_render] = (float)glfwGetTime();
  while (pl_list != NULL || dl_list != NULL) {
    glUniform1i(glGetUniformLocation(s->shader, "u_ambient_pass"), ambient_pass);
    ambient_pass = 0;
    
    // point light
    if (pl_list != NULL && pl_list->data != NULL) {
      point_light_t *pl = pl_list->data;
      point_light_draw(pl, s->shader);
      glUniform1i(glGetUniformLocation(s->shader, "u_point_active"), 1);
    } else {
      glUniform1i(glGetUniformLocation(s->shader, "u_point_active"), 0);
    }

    // dir light
    if (dl_list != NULL && dl_list->data != NULL) {
      dir_light_t *dl = dl_list->data;
      dir_light_draw(dl, s->shader);
      glUniform1i(glGetUniformLocation(s->shader, "u_dir_active"), 1);
    } else {
      glUniform1i(glGetUniformLocation(s->shader, "u_dir_active"), 0);
    }

    // render models
    scene_render_models(s, s->shader, 0);

    // enable blending for second pass onwards
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    if (dl_list != NULL && dl_list->next != NULL)
      dl_list = dl_list->next;
    else
      dl_list = NULL;

    if (pl_list != NULL && pl_list->next != NULL)
      pl_list = pl_list->next;
    else
      pl_list = NULL;

    if (pl_list == NULL && dl_list == NULL)
      break;
  }
  glDisable(GL_BLEND);
  ex_dbgprofiler.end[ex_dbgprofiler_lighting_render] = (float)glfwGetTime();

  // update camera
  ex_dbgprofiler.begin[ex_dbgprofiler_camera] = (float)glfwGetTime();
  if (s->fps_camera != NULL) {
    fps_camera_update(s->fps_camera, s->shader);
    fps_camera_draw(s->fps_camera, s->shader);
  }
  ex_dbgprofiler.end[ex_dbgprofiler_camera] = (float)glfwGetTime();
  */
 
  // render screen quad
  // framebuffer_render_quad();
  ex_dbgprofiler.begin[ex_dbgprofiler_other] = (float)glfwGetTime();
}

void scene_render_models(scene_t *s, GLuint shader, int shadows)
{
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    model_t *m = n->data;

    if ((shadows && m->is_shadow) || !shadows)
      model_draw(m, shader);

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
  texture_t *t = texture_load(file, 0);
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

  // cleanup skybox
  if (s->skybox != NULL) {
    skybox_destroy(s->skybox);
  }

  // cleanup framebuffers
  framebuffer_destroy();
}