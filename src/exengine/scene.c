#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "model.h"
#include "pointlight.h"
#include "dirlight.h"
#include "framebuffer.h"
#include "gbuffer.h"
#include "window.h"

scene_t* scene_new()
{
  scene_t *s = malloc(sizeof(scene_t));

  // init lists
  s->model_list       = list_new();
  s->texture_list     = list_new();

  s->fps_camera = NULL;

  // init framebuffers etc
  framebuffer_init();
  gbuffer_init();

  // init lights
  point_light_init();
  dir_light_init();
  s->dir_light = NULL;
  for (int i=0; i<MAX_POINT_LIGHTS; i++)
    s->point_lights[i] = NULL;

  // init skybox
  s->skybox = NULL;
  skybox_init();

  // init physics shiz
  memset(s->gravity, 0, sizeof(vec3));

  // init debug gui
  ex_dbgui_init();

  // init debug vars
  s->plightc    = 0;
  s->dynplightc = 0;
  s->dlightc    = 0;
  s->modelc     = 0;

  return s;
}

void scene_add_pointlight(scene_t *s, point_light_t *pl)
{
  if (pl->dynamic && pl->is_shadow)
    s->dynplightc++;
  else
    s->plightc++;

  for (int i=0; i<MAX_POINT_LIGHTS; i++) {
    if (s->point_lights[i] == NULL) {
      s->point_lights[i] = pl;
      return;
    }
  }

  printf("Maximum point lights exceeded!\n");
}

void scene_update(scene_t *s, float delta_time)
{
  ex_dbgprofiler.begin[ex_dbgprofiler_update] = (float)glfwGetTime();

  // update models animations etc
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    model_update(n->data, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // handle light stuffs
  scene_manage_lights(s);

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
  for (int i=0; i<MAX_POINT_LIGHTS; i++) {
    point_light_t *l = s->point_lights[i];
    if (l != NULL && (l->dynamic || l->update) && l->is_shadow && l->is_visible) {
      point_light_begin(l);
      scene_render_models(s, l->shader, 1);
    }
  }

  // render dirlight depth maps
  glCullFace(GL_BACK);
  if (s->dir_light != NULL) {
    dir_light_t *l = s->dir_light;
    if (s->fps_camera != NULL) {
      memcpy(l->cposition, s->fps_camera->position, sizeof(vec3));
      l->cposition[1] = 0.0f;
    }
    dir_light_begin(l);
    scene_render_models(s, l->shader, 1);
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

  // render scene to gbuffer
  fps_camera_draw(s->fps_camera, gshader);
  scene_render_models(s, gshader, 0);

  framebuffer_first();

  // render skybox (FIX THIS!)
  /*if (s->skybox != NULL) {
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(s->skybox->shader);

    if (s->fps_camera != NULL)
      fps_camera_draw(s->fps_camera, s->skybox->shader);
    
    skybox_draw(s->skybox);
  }*/

  // render scene
  glUseProgram(gmainshader);

  // update camera
  if (s->fps_camera != NULL) {
    fps_camera_update(s->fps_camera, gmainshader);
    fps_camera_draw(s->fps_camera, gmainshader);
  }

  // first pass is ambient
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);

  glUniform1i(glGetUniformLocation(gmainshader, "u_ambient_pass"), 1);
  glUniform1i(glGetUniformLocation(gmainshader, "u_point_active"), 0);
  glUniform1i(glGetUniformLocation(gmainshader, "u_dir_active"), 0);
  gbuffer_render(gmainshader);

  // enable blending for second pass onwards
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // do all non shadow casting lights in a single pass
  // including the one directional light
  // and lights outside of the shadow render range
  int count = 0;
  char buff[64];
  for (int i=0; i<MAX_POINT_LIGHTS; i++) {
    point_light_t *pl = s->point_lights[i];
    if (pl == NULL || !pl->is_visible)
      continue;

    if (!pl->is_shadow || pl->distance_to_cam > POINT_SHADOW_DIST) {
      sprintf(buff, "u_point_lights[%d]", count);
      point_light_draw(pl, gmainshader, buff);
      count++;
    }
  }

  if (s->dir_light != NULL) {
    dir_light_draw(s->dir_light, gmainshader);
    glUniform1i(glGetUniformLocation(gmainshader, "u_dir_active"), 1);
  }

  glUniform1i(glGetUniformLocation(gmainshader, "u_point_count"), count);
  glUniform1i(glGetUniformLocation(gmainshader, "u_ambient_pass"), 0);
  gbuffer_render(gmainshader);
  glUniform1i(glGetUniformLocation(gmainshader, "u_point_count"), 0);
  glUniform1i(glGetUniformLocation(gmainshader, "u_dir_active"), 0);

  // enable blending for second pass onwards
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // render all shadow casting point lights
  ex_dbgprofiler.begin[ex_dbgprofiler_lighting_render] = (float)glfwGetTime();
  for (int i=0; i<MAX_POINT_LIGHTS; i++) {
    point_light_t *pl = s->point_lights[i];
    if (pl == NULL || !pl->is_visible)
      continue;

    if (pl->is_shadow && pl->distance_to_cam <= POINT_SHADOW_DIST) {
      glUniform1i(glGetUniformLocation(gmainshader, "u_point_active"), 1);
      point_light_draw(pl, gmainshader, NULL);
    } else {
      glUniform1i(glGetUniformLocation(gmainshader, "u_point_active"), 0);
    }

    gbuffer_render(gmainshader);
  }
  glDisable(GL_BLEND);
  ex_dbgprofiler.end[ex_dbgprofiler_lighting_render] = (float)glfwGetTime();
 
  // render screen quad
  framebuffer_render_quad();
  ex_dbgprofiler.begin[ex_dbgprofiler_other] = (float)glfwGetTime();
}

void scene_manage_lights(scene_t *s)
{
  // set our position and front vector
  vec3 thispos, thisfront;
  if (s->fps_camera != NULL) {
    memcpy(thisfront, s->fps_camera->front, sizeof(vec3));
    memcpy(thispos, s->fps_camera->position, sizeof(vec3));
  }

  // point lights
  for (int i=0; i<MAX_POINT_LIGHTS; i++) {
    point_light_t *pl = s->point_lights[i];
    if (pl == NULL)
      continue;

    // direction to light
    vec3 thatpos;
    vec3_sub(thatpos, pl->position, thispos);
    pl->distance_to_cam = vec3_len(thatpos);
    vec3_norm(thatpos, thatpos);
    vec3_norm(thisfront, thisfront);

    // dot to light
    float f = vec3_mul_inner(thisfront, thatpos);

    // check if its behind us and far away
    if (f <= 0.1f && pl->distance_to_cam > POINT_FAR_PLANE)
      pl->is_visible = 0;
    else
      pl->is_visible = 1;
  }
}

void scene_dbgui(scene_t *s)
{
  if (s->dir_light != NULL)
    s->dlightc = 1;

  int rendered_lights = 0;
  int culled_lights = 0;
  for (int i=0; i<MAX_POINT_LIGHTS; i++)
    if (s->point_lights[i] != NULL && s->point_lights[i]->is_visible)
      rendered_lights++;
    else if (s->point_lights[i] != NULL)
      culled_lights++;

  // draw gui
  if (igBegin("Scene Debugger", NULL, ImGuiWindowFlags_NoTitleBar)) {
    igColumns(3, "", 0);
    igText("Item Type");
    igNextColumn();
    igText("Count");
    igNextColumn();
    igText("Specifics");
    igSeparator();
    igColumns(3, "", 0);
    igText("Point Lights");
    igText("Point Lights");
    igText("Directional Lights");
    igText("Rendered Lights");
    igText("Culled Lights");
    igText("Scene Models");
    igNextColumn();
    igText("%i", s->dynplightc);
    igText("%i", s->plightc);
    igText("%i", s->dlightc);
    igText("%i", rendered_lights);
    igText("%i", culled_lights);
    igText("%i", s->modelc);
    igNextColumn();
    igText("Dynamic");
    igText("Static");
    igText("Dynamic");
    igText("");
  }

  igEnd();
}

void scene_render_models(scene_t *s, GLuint shader, int shadows)
{
  s->modelc = 0;
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    model_t *m = n->data;
    s->modelc++;

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
  for (int i=0; i<MAX_POINT_LIGHTS; i++) {
    if (s->point_lights[i] != NULL) {
      point_light_destroy(s->point_lights[i]);
    }
  }

  // clean up dir lights
  if (s->dir_light != NULL)
    dir_light_destroy(s->dir_light);

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