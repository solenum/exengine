#ifndef EX_RENDERER_H
#define EX_RENDERER_H

#include <stdlib.h>
#include "pointlight.h"
#include "model.h"
#include "math/mathlib.h"

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
  ex_renderlist_t *models;
  ex_renderlist_t *point_lights;
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

static inline void ex_rendernode_pop(ex_renderlist_t *list, ex_rendernode_t *node) {
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

void ex_render(ex_renderer_e renderer, ex_renderable_t *renderables);

void ex_render_forward(ex_renderable_t *renderables);

void ex_render_model(ex_model_t *model, GLuint shader);

void ex_render_mesh(ex_mesh_t *mesh, GLuint shader, size_t count);

void ex_render_point_light_begin(ex_point_light_t *light, GLuint shader);

#endif // EX_RENDERER_H