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
#include "ssao.h"

ex_scene_t* scene_new()
{
  ex_scene_t *s = malloc(sizeof(ex_scene_t));

  // init lists
  s->model_list       = list_new();
  s->texture_list     = list_new();

  s->fps_camera = NULL;

  // init framebuffers etc
  ex_framebuffer_init();
  ex_gbuffer_init();

  // init lights
  ex_point_light_init();
  ex_dir_light_init();
  ex_spot_light_init();
  s->dir_light = NULL;
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++)
    s->point_lights[i] = NULL;
  for (int i=0; i<EX_MAX_SPOT_LIGHTS; i++)
    s->spot_lights[i] = NULL;

  // init skybox
  s->skybox = NULL;
  ex_skybox_init();

  // init physics shiz
  memset(s->gravity, 0, sizeof(vec3));
  s->coll_tree = ex_octree_new(OBJ_TYPE_UINT);
  s->coll_list = list_new();
  s->coll_vertices   = NULL;
  s->collision_built = 0;
  s->coll_vertices_last = 0;
  memset(s->coll_tree->region.min, 0, sizeof(vec3));
  memset(s->coll_tree->region.max, 0, sizeof(vec3));

  // init debug gui
  ex_dbgui_init(s);

  // init debug vars
  s->plightc    = 0;
  s->dynplightc = 0;
  s->dlightc    = 0;
  s->slightc    = 0;
  s->modelc     = 0;

  // primitive debug shader
  s->primshader = ex_shader_compile("primshader.vs", "primshader.fs", NULL);

  // init ssao stuffs
  ssao_init();

  return s;
}

void ex_scene_add_collision(ex_scene_t *s, ex_model_t *model)
{
  if (model != NULL) {
    if (model->vertices != NULL && model->num_vertices > 0) {
      list_add(s->coll_list, (void*)model);
      s->collision_built = 0;

      if (s->coll_vertices != NULL) {
        size_t len = model->num_vertices + s->coll_vertices_last;
        s->coll_vertices = realloc(s->coll_vertices, sizeof(vec3)*len);
        memcpy(&s->coll_vertices[s->coll_vertices_last], &model->vertices[0], sizeof(vec3)*model->num_vertices);
        s->coll_vertices_last = len;
      } else {
        s->coll_vertices = malloc(sizeof(vec3)*model->num_vertices);
        memcpy(&s->coll_vertices[0], &model->vertices[0], sizeof(vec3)*model->num_vertices);
        s->coll_vertices_last = model->num_vertices;
      }

      free(model->vertices);
      model->vertices     = NULL;
      model->num_vertices = 0;
      s->collision_built  = 0;
    }
  }
}

void ex_scene_build_collision(ex_scene_t *s)
{
  // destroy and reconstruct tree
  if (s->coll_tree->built)
    s->coll_tree = ex_octree_reset(s->coll_tree);

  if (s->coll_tree == NULL || s->coll_vertices == NULL || s->coll_vertices_last == 0)
    return;

  ex_rect_t region;
  memcpy(&region, &s->coll_tree->region, sizeof(ex_rect_t));
  for (int i=0; i<s->coll_vertices_last; i+=3) {
    vec3 tri[3];
    memcpy(tri[0], s->coll_vertices[i+0], sizeof(vec3));
    memcpy(tri[1], s->coll_vertices[i+1], sizeof(vec3));
    memcpy(tri[2], s->coll_vertices[i+2], sizeof(vec3));

    vec3_min(region.min, region.min, tri[0]);
    vec3_min(region.min, region.min, tri[1]);
    vec3_min(region.min, region.min, tri[2]);
    vec3_max(region.max, region.max, tri[0]);
    vec3_max(region.max, region.max, tri[1]);
    vec3_max(region.max, region.max, tri[2]);

    ex_octree_obj_t *obj = malloc(sizeof(ex_octree_obj_t));

    obj->data_uint    = i;
    obj->box          = ex_rect_from_triangle(tri);
    list_add(s->coll_tree->obj_list, (void*)obj);
  }

  memcpy(&s->coll_tree->region, &region, sizeof(ex_rect_t));
  ex_octree_build(s->coll_tree);

  s->collision_built = 1;
}

void ex_scene_add_pointlight(ex_scene_t *s, ex_point_light_t *pl)
{
  if (pl->dynamic && pl->is_shadow)
    s->dynplightc++;
  else
    s->plightc++;

  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    if (s->point_lights[i] == NULL) {
      s->point_lights[i] = pl;
      return;
    }
  }

  printf("Maximum point lights exceeded!\n");
}

void ex_scene_add_spotlight(ex_scene_t *s, ex_spot_light_t *sl)
{
  if (sl->dynamic && sl->is_shadow)
    s->dynplightc++;
  else
    s->slightc++;

  for (int i=0; i<EX_MAX_SPOT_LIGHTS; i++) {
    if (s->spot_lights[i] == NULL) {
      s->spot_lights[i] = sl;
      return;
    }
  }
}

void ex_scene_update(ex_scene_t *s, float delta_time)
{
  ex_dbgprofiler.begin[ex_dbgprofiler_update] = glfwGetTime();

  if (!s->collision_built)
    ex_scene_build_collision(s);

  // update models animations etc
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    ex_model_update(n->data, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // handle light stuffs
  ex_scene_manage_lights(s);

  ex_dbgprofiler.end[ex_dbgprofiler_update] = glfwGetTime();
}

void ex_scene_draw(ex_scene_t *s)
{
  // begin profiler
  ex_dbgprofiler.end[ex_dbgprofiler_other] = glfwGetTime();
  ex_dbgui_end_profiler();
  ex_dbgui_begin_profiler();

  // render pointlight depth maps
  glCullFace(GL_BACK);
  ex_dbgprofiler.begin[ex_dbgprofiler_lighting_depth] = glfwGetTime();
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *l = s->point_lights[i];
    if (l != NULL && (l->dynamic || l->update) && l->is_shadow && l->is_visible) {
      ex_point_light_begin(l);
      ex_scene_render_models(s, l->shader, 1);
    }
  }

  // render spotlight depth maps
  for (int i=0; i<EX_MAX_SPOT_LIGHTS; i++) {
    ex_spot_light_t *l = s->spot_lights[i];
    if (l != NULL && (l->dynamic || l->update) && l->is_shadow && l->is_visible) {
      ex_spot_light_begin(l);
      ex_scene_render_models(s, l->shader, 1);
    }
  }

  // render dirlight depth maps
  glCullFace(GL_BACK);
  if (s->dir_light != NULL) {
    ex_dir_light_t *l = s->dir_light;
    if (s->fps_camera != NULL) {
      memcpy(l->cposition, s->fps_camera->position, sizeof(vec3));
      l->cposition[1] = 0.0f;
    }
    ex_dir_light_begin(l);
    ex_scene_render_models(s, l->shader, 1);
  }
  ex_dbgprofiler.end[ex_dbgprofiler_lighting_depth] = glfwGetTime();

  ex_fps_camera_update(s->fps_camera, ex_gshader);

  // first geometry render pass
  ex_gbuffer_first();
  glUseProgram(ex_gshader);
  
  // debug poooo
  if (ex_keys_down[GLFW_KEY_E])
    glUniform1i(glGetUniformLocation(ex_gshader, "u_dont_norm"), 0);
  if (ex_keys_down[GLFW_KEY_R])
    glUniform1i(glGetUniformLocation(ex_gshader, "u_dont_norm"), 1);

  // render scene to gbuffer
  ex_fps_camera_draw(s->fps_camera, ex_gshader);
  ex_scene_render_models(s, ex_gshader, 0);

  // render ssao
  ssao_render(s->fps_camera->projection, s->fps_camera->view);

  ex_framebuffer_first();

  // render skybox (FIX THIS!)
  /*if (s->skybox != NULL) {
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(s->skybox->shader);

    if (s->fps_camera != NULL)
      ex_fps_camera_draw(s->fps_camera, s->skybox->shader);
    
    ex_skybox_draw(s->skybox);
  }*/

  // render scene
  glUseProgram(ex_gmainshader);

  // update camera
  if (s->fps_camera != NULL) {
    ex_fps_camera_update(s->fps_camera, ex_gmainshader);
    ex_fps_camera_draw(s->fps_camera, ex_gmainshader);
  }

  // first pass is ambient
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);

  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_ambient_pass"), 1);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_point_active"), 0);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_dir_active"), 0);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_spot_active"), 0);
  ex_gbuffer_render(ex_gmainshader);

  // enable blending for second pass onwards
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // do all non shadow casting lights in a single pass
  // including the one directional light
  // and lights outside of the shadow render range
  int pcount = 0;
  char buff[64];
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *pl = s->point_lights[i];
    if (pl == NULL || !pl->is_visible)
      continue;

    if (!pl->is_shadow || pl->distance_to_cam > EX_POINT_SHADOW_DIST) {
      sprintf(buff, "u_point_lights[%d]", pcount);
      ex_point_light_draw(pl, ex_gmainshader, buff);
      pcount++;
    }
  }

  // render non shadow casting spot lights
  int scount = 0;
  for (int i=0; i<EX_MAX_SPOT_LIGHTS; i++) {
    ex_spot_light_t *sl = s->spot_lights[i];
    if (sl == NULL || !sl->is_visible)
      continue;

    if (!sl->is_shadow || sl->distance_to_cam > EX_SPOT_SHADOW_DIST) {
      sprintf(buff, "u_spot_lights[%d]", scount);
      ex_spot_light_draw(sl, ex_gmainshader, buff);
      scount++;
    }
  }

  if (s->dir_light != NULL) {
    ex_dir_light_draw(s->dir_light, ex_gmainshader);
    glUniform1i(glGetUniformLocation(ex_gmainshader, "u_dir_active"), 1);
  }

  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_point_count"), pcount);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_spot_count"), scount);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_ambient_pass"), 0);
  ex_gbuffer_render(ex_gmainshader);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_point_count"), 0);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_spot_count"), 0);
  glUniform1i(glGetUniformLocation(ex_gmainshader, "u_dir_active"), 0);

  // enable blending for second pass onwards
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // render all shadow casting point lights
  ex_dbgprofiler.begin[ex_dbgprofiler_lighting_render] = glfwGetTime();
  for (int i=0; i<EX_SCENE_BIGGEST_LIGHT; i++) {
    ex_point_light_t *pl = i > EX_MAX_POINT_LIGHTS ? NULL : s->point_lights[i];
    ex_spot_light_t  *sl = i > EX_MAX_SPOT_LIGHTS ? NULL : s->spot_lights[i];
    
    if (pl == NULL && sl == NULL)
      continue;

    // point light
    if (pl != NULL) {
      if (pl->is_shadow && pl->distance_to_cam <= EX_POINT_SHADOW_DIST && pl->is_visible) {
        glUniform1i(glGetUniformLocation(ex_gmainshader, "u_point_active"), 1);
        ex_point_light_draw(pl, ex_gmainshader, NULL);
      } else {
        glUniform1i(glGetUniformLocation(ex_gmainshader, "u_point_active"), 0);
      } 
    }

    // spot light
    if (sl != NULL) {
      if (sl->is_shadow && sl->distance_to_cam <= EX_SPOT_SHADOW_DIST && sl->is_visible) {
        glUniform1i(glGetUniformLocation(ex_gmainshader, "u_spot_active"), 1);
        ex_spot_light_draw(sl, ex_gmainshader, NULL);
      } else {
        glUniform1i(glGetUniformLocation(ex_gmainshader, "u_spot_active"), 0);
      } 
    }

    // render gbuffer to screen quad
    if (pl != NULL || sl != NULL)
      ex_gbuffer_render(ex_gmainshader);
  }
  glDisable(GL_BLEND);
  ex_dbgprofiler.end[ex_dbgprofiler_lighting_render] = glfwGetTime();

  // render debug primitives
  glUseProgram(s->primshader);

  // update camera
  if (s->fps_camera != NULL)
    ex_fps_camera_draw(s->fps_camera, s->primshader);

  if (ex_dbgprofiler.render_octree)
    ex_octree_render(s->coll_tree);

  // render screen quad
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ex_framebuffer_render_quad();

  ex_dbgprofiler.begin[ex_dbgprofiler_other] = glfwGetTime();
}

void ex_scene_manage_lights(ex_scene_t *s)
{
  // set our position and front vector
  vec3 thispos, thisfront;
  if (s->fps_camera != NULL) {
    memcpy(thisfront, s->fps_camera->front, sizeof(vec3));
    memcpy(thispos, s->fps_camera->position, sizeof(vec3));
  }

  // point lights
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *pl = s->point_lights[i];
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
    if (f <= -0.5f && pl->distance_to_cam > EX_POINT_FAR_PLANE)
      pl->is_visible = 0;
    else
      pl->is_visible = 1;
  }

  // spot lights
  for (int i=0; i<EX_MAX_SPOT_LIGHTS; i++) {
    ex_spot_light_t *sl = s->spot_lights[i];
    if (sl == NULL)
      continue;

    // direction to light
    vec3 thatpos;
    vec3_sub(thatpos, sl->position, thispos);
    sl->distance_to_cam = vec3_len(thatpos);
    vec3_norm(thatpos, thatpos);
    vec3_norm(thisfront, thisfront);

    // dot to light
    float f = vec3_mul_inner(thisfront, thatpos);

    // check if its behind us and far away
    if (f <= -0.5f && sl->distance_to_cam > EX_SPOT_FAR_PLANE)
      sl->is_visible = 0;
    else
      sl->is_visible = 1;
  }
}

void ex_scene_dbgui(ex_scene_t *s)
{
  if (s->dir_light != NULL)
    s->dlightc = 1;

  int rendered_lights = 0;
  int culled_lights = 0;
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++)
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

void ex_scene_render_models(ex_scene_t *s, GLuint shader, int shadows)
{
  if (ex_dbgprofiler.wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  s->modelc = 0;
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    ex_model_t *m = (ex_model_t*)n->data;
    s->modelc++;

    if ((shadows && m->is_shadow) || !shadows)
      ex_model_draw(m, shader);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

GLuint ex_scene_add_texture(ex_scene_t *s, const char *file)
{  
  // check if texture already exists
  list_node_t *n = s->texture_list;
  while (n->data != NULL) {
    ex_texture_t *t = n->data;

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
  ex_texture_t *t = ex_texture_load(file, 0);
  if (t != NULL) {
    // store it in the list
    list_add(s->texture_list, (void*)t);
    return t->id;
  }

  return 0;
}

void ex_scene_destroy(ex_scene_t *s)
{
  printf("Cleaning up scene\n");

  // cleanup models
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    ex_model_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free model list
  list_destroy(s->model_list);

  // cleanup point lights
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    if (s->point_lights[i] != NULL) {
      ex_point_light_destroy(s->point_lights[i]);
    }
  }

  // clean up dir lights
  if (s->dir_light != NULL)
    ex_dir_light_destroy(s->dir_light);

  // cleanup textures
  n = s->texture_list;
  while (n->data != NULL) {
    ex_texture_t *t = n->data;
    
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
    ex_skybox_destroy(s->skybox);
  }

  // cleanup framebuffers
  ex_framebuffer_destroy();
}