/* renderer
  Handles the rendering pipeline, each
  rendering pass etc.

  The renderer requires that you construct
  and manage your own list of renderable
  objects in a ex_renderable_t struct.

  The struct contains an individual list
  per type of thing being rendered.

  Those lists themselves contain a list of
  render nodes, which are void pointers to
  the object that needs to be rendered along
  with some metadata like rectangle bounds
  for culling, etc.
*/
#ifndef EX_RENDERER_H
#define EX_RENDERER_H

#include "pointlight.h"
#include "model.h"
#include "camera.h"
#include "math/mathlib.h"

#include "glad/glad.h"
#include <stdlib.h>

/*
  how much the render list increases
  or decreases when it starts to overflow
  with data.
*/
#define EX_RENDER_LIST_INC 32

typedef enum {
  EX_RENDERER_FORWARD,
  EX_RENDERER_DEFERRED
} ex_renderer_e;

typedef struct {
  void *obj;
  rect_t bounds;
  size_t index;
} ex_rendernode_t;

typedef struct {
  ex_rendernode_t *nodes;
  size_t length, count;
} ex_renderlist_t;

typedef struct {
  ex_renderlist_t models;
  ex_renderlist_t point_lights;
  ex_camera_matrices_t *camera;
} ex_renderable_t;

/**
 * [ex_rendernode_push push a new render node to a list]
 * @param  list [list that will be added to]
 * @return      [pointer to new node added]
 */
static inline ex_rendernode_t *ex_rendernode_push(ex_renderlist_t *list) {
  if (list->count == list->length) {
    list->length += EX_RENDER_LIST_INC;
    size_t len = sizeof(ex_rendernode_t) * list->length;
    ex_rendernode_t *nodes = realloc(list->nodes, len);

    if (nodes)
      list->nodes = nodes;
  }

  ex_rendernode_t *node = &list->nodes[list->count];
  node->index = list->count;
  list->count++;

  return node;
}

/**
 * [ex_rendernode_pop pop a render node from a list]
 * @param list [list to pop from]
 * @param obj  [object pointer to pop]
 *
 * you still need to free the object yourself after!
 */
static inline void ex_rendernode_pop(ex_renderlist_t *list, void *obj) {
  ex_rendernode_t *node = NULL;

  for (int i=0; i<list->count; i++) {
    if (list->nodes[i].obj == obj) {
      node = &list->nodes[i];
      break;
    }
  }

  if (!node)
    return;

  // node is somewhere middle of list, pop from list
  if (node->index < list->count) {
    size_t index = node->index;
    for (int i=index+1; i<list->count; i++)
      list->nodes[index++] = list->nodes[i];
  
  }
  list->count--;

  // realloc if count is considerably lower than count
  if (list->count < list->length-EX_RENDER_LIST_INC) {
    list->length -= EX_RENDER_LIST_INC;
    size_t len = sizeof(ex_rendernode_t) * list->length;
    ex_rendernode_t *nodes = realloc(list->nodes, len);

    if (nodes)
      list->nodes = nodes;
  }
}

/**
 * [ex_render_init sets up internal rendering stuffs]
 */
void ex_render_init();

/**
 * [ex_render runs a single render pass]
 * @param renderer    [renderer instance to use]
 * @param renderables [list of renderables and nodes]
 */
void ex_render(ex_renderer_e renderer, ex_renderable_t *renderables);

/**
 * [ex_render_forward forward renderer pass]
 * @param renderables [list of renderables and nodes]
 */
void ex_render_forward(ex_renderable_t *renderables);

/**
 * [ex_render_model renders a single model]
 * @param model  [model to render]
 * @param shader [shader to use]
 */
void ex_render_model(ex_model_t *model, GLuint shader);

/**
 * [ex_render_mesh renders a single mesh or instanced meshes]
 * @param mesh   [mesh to render]
 * @param shader [shader to use]
 * @param count  [instances to render]
 */
void ex_render_mesh(ex_mesh_t *mesh, GLuint shader, size_t count);

/**
 * [ex_render_point_light_begin bind a pointlight]
 * @param light  [pointlight to use]
 * @param shader [shader to use]
 */
void ex_render_point_light_begin(ex_point_light_t *light, GLuint shader);

/**
 * [ex_render_point_light renders a single pointlight]
 * @param light  [pointlight to use]
 * @param shader [shader to use]
 * @param prefix [prefix to use for shader uniform]
 */
void ex_render_point_light(ex_point_light_t *light, GLuint shader, const char *prefix);

/**
 * [ex_render_resize resizes internal framebuffers etc]
 * @param width  [the new width]
 * @param height [the new height]
 */
void ex_render_resize(size_t width, size_t height);

/**
 * [ex_render_destroy cleans up data]
 */
void ex_render_destroy();

#endif // EX_RENDERER_H