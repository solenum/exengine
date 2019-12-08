#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "sound/sound.h"

ex_scene_t* ex_scene_new(uint8_t flags)
{
  ex_scene_t *s = malloc(sizeof(ex_scene_t));

  // init physics shiz
  memset(s->gravity, 0, sizeof(vec3));
  s->coll_tree = ex_octree_new(OBJ_TYPE_UINT);
  s->coll_list = ex_list_new();
  s->coll_vertices   = NULL;
  s->collision_built = 0;
  s->coll_vertices_last = 0;
  memset(s->coll_tree->region.min, 0, sizeof(vec3));
  memset(s->coll_tree->region.max, 0, sizeof(vec3));

  // init renderables list
  s->renderables.models.nodes  = malloc(sizeof(ex_rendernode_t) * 32);
  s->renderables.models.count  = 0;
  s->renderables.models.length = 0;

  s->renderables.point_lights.nodes = malloc(sizeof(ex_rendernode_t) * 32);
  s->renderables.point_lights.count  = 0;
  s->renderables.point_lights.length = 0;

  ex_render_init();

  return s;
}

void ex_scene_add_collision(ex_scene_t *s, ex_model_t *model)
{
  if (model != NULL) {
    if (model->vertices != NULL && model->num_vertices > 0) {
      ex_list_add(s->coll_list, (void*)model);
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

  rect_t region;
  memcpy(&region, &s->coll_tree->region, sizeof(rect_t));
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
    ex_list_add(s->coll_tree->obj_list, (void*)obj);
  }

  memcpy(&s->coll_tree->region, &region, sizeof(rect_t));
  ex_octree_build(s->coll_tree);

  s->collision_built = 1;
}

void ex_scene_add_model(ex_scene_t *s, ex_model_t *m)
{
  ex_rendernode_t *node = ex_rendernode_push(&s->renderables.models);
  node->obj = (void*)m;
}

void ex_scene_remove_model(ex_scene_t *s, ex_model_t *m)
{
  ex_rendernode_pop(&s->renderables.models, (void*)m);
}

void ex_scene_add_pointlight(ex_scene_t *s, ex_point_light_t *pl)
{
  ex_rendernode_t *node = ex_rendernode_push(&s->renderables.point_lights);
  node->obj = (void*)pl;
}

void ex_scene_update(ex_scene_t *s, float delta_time)
{
  if (!s->collision_built)
    ex_scene_build_collision(s);

  // update models animations etc
  for (int i=0; i<s->renderables.models.count; i++) {
    ex_model_t *model = (ex_model_t*)s->renderables.models.nodes[i].obj;
    ex_model_update(model, delta_time);
  }
}

void ex_scene_draw(ex_scene_t *s, int view_x, int view_y, int view_width, int view_height, ex_camera_matrices_t *matrices)
{
  s->renderables.camera = matrices;

  ex_render(EX_RENDERER_FORWARD, &s->renderables);
}

void ex_scene_resize(ex_scene_t *s, int width, int height)
{
  ex_render_resize(width, height);
}

void ex_scene_destroy(ex_scene_t *s)
{
  printf("Cleaning up scene\n");

  for (int i=0; i<s->renderables.point_lights.count; i++) {  
    ex_point_light_t *light = (ex_point_light_t*)s->renderables.point_lights.nodes[i].obj;
    ex_point_light_destroy(light);
  }

  free(s->renderables.models.nodes);
  free(s->renderables.point_lights.nodes);

  ex_render_destroy();
}