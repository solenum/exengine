#include "math/octree.h"
#include "util/vertices.h"
#include <stdio.h>

int ex_octree_min_size = EX_OCTREE_DEFAULT_MIN_SIZE;

ex_octree_t* ex_octree_new(uint8_t type)
{
  ex_octree_t *o = malloc(sizeof(ex_octree_t));

  for (int i=0; i<8; i++) {
    o->children[i] = NULL;
  }
  memset(o->region.min, 0.0f, sizeof(vec3));
  memset(o->region.max, 1.0f, sizeof(vec3));

  o->rendered = 0;
  o->built    = 0;
  o->first    = 1;
  o->obj_list = ex_list_new();
  ex_octree_min_size = ex_octree_min_size;

  o->data_len    = 0;
  o->data_type   = type;
  o->data_uint   = NULL;
  o->data_int    = NULL;
  o->data_byte   = NULL;
  o->data_float  = NULL;
  o->data_double = NULL;

  return o;
}

void ex_octree_init(ex_octree_t *o, rect_t region, ex_list_t *objects)
{
  memcpy(&o->region, &region, sizeof(rect_t));
  for (int i=0; i<8; i++) {
    o->children[i] = NULL;
  }
  o->obj_list    = objects;
  o->rendered    = 0;
  o->built       = 0;
  o->first       = 0;
  o->data_len    = 0;
  o->data_type   = OBJ_TYPE_NULL;
  o->data_uint   = NULL;
  o->data_int    = NULL;
  o->data_byte   = NULL;
  o->data_float  = NULL;
  o->data_double = NULL;
}

void ex_octree_build(ex_octree_t *o)
{
  if (o->obj_list->data == NULL)
    return;

  if (o->obj_list->next == NULL) {
    ex_octree_finalize(o);
    return;
  }

  // our size
  vec3 region;
  vec3_sub(region, o->region.max, o->region.min);

  if (region[0] <= ex_octree_min_size || region[1] <= ex_octree_min_size || region[2] <= ex_octree_min_size) {
    if (!o->first) {
      ex_octree_finalize(o);
      return;
    }
  }

  vec3 half, center;
  vec3_scale(half, region, 0.5f);
  vec3_add(center, o->region.min, half);

  // octant regions
  rect_t octants[8];
  octants[0] = ex_rect_new(o->region.min, center);
  octants[1] = ex_rect_new((vec3){center[0], o->region.min[1], o->region.min[2]}, (vec3){o->region.max[0], center[1], center[2]});
  octants[2] = ex_rect_new((vec3){center[0], o->region.min[1], center[2]}, (vec3){o->region.max[0], center[1], o->region.max[2]});
  octants[3] = ex_rect_new((vec3){o->region.min[0], o->region.min[1], center[2]}, (vec3){center[0], center[1], o->region.max[2]});
  octants[4] = ex_rect_new((vec3){o->region.min[0], center[1], o->region.min[2]}, (vec3){center[0], o->region.max[1], center[2]});
  octants[5] = ex_rect_new((vec3){center[0], center[1], o->region.min[2]}, (vec3){o->region.max[0], o->region.max[1], center[2]});
  octants[6] = ex_rect_new(center, o->region.max);
  octants[7] = ex_rect_new((vec3){o->region.min[0], center[1], center[2]}, (vec3){center[0], o->region.max[1], o->region.max[2]});

  // object lists
  ex_list_t *obj_lists[8];
  size_t  obj_lenghts[8];
  for (int i=0; i<8; i++) {
    obj_lists[i]   = ex_list_new();
    obj_lenghts[i] = 0;
  }

  // add objects to appropriate octant
  size_t obj_count = 0;
  ex_list_node_t *n = o->obj_list;
  while (n->data != NULL) {
    int found = 0;

    for (int j=0; j<8; j++) {
      ex_octree_obj_t *obj = (ex_octree_obj_t*)n->data;
      if (ex_aabb_inside(octants[j], obj->box)) {
        ex_list_add(obj_lists[j], (void*)n->data);
        obj_lenghts[j]++;
        found = 1;
        break;
      }
    }

    // remove obj from this list
    if (found) {
      ex_list_t *next = n->next;
      o->obj_list = ex_list_remove(o->obj_list, (void*)n->data);
      if (next != NULL) {
        n = next;
        continue;
      } else {
        break;
      }
    } else {
      obj_count++;
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // create children
  for (int i=0; i<8; i++) {
    if (obj_lists[i]->data != NULL) {
      o->children[i] = malloc(sizeof(ex_octree_t));
      ex_octree_init(o->children[i], octants[i], obj_lists[i]);
      o->children[i]->data_len  = obj_lenghts[i];
      o->children[i]->data_type = o->data_type;
      ex_octree_build(o->children[i]);
    } else {
      o->children[i] = NULL;
    }
  }

  o->data_len = obj_count;
  ex_octree_finalize(o);
}

void ex_octree_finalize(ex_octree_t *o)
{
  // move object data into a flat array
  int i = 0;
  ex_list_node_t *n = o->obj_list;
  while (n->data != NULL) {
    ex_octree_obj_t *data = n->data;

    switch (o->data_type) {
      case OBJ_TYPE_UINT:
        if (i == 0)
          o->data_uint   = malloc(o->data_len * sizeof(uint32_t));
        memcpy(&o->data_uint[i], &data->data_uint, sizeof(uint32_t));
        break;
      case OBJ_TYPE_INT:
        if (i == 0)
          o->data_int    = malloc(o->data_len * sizeof(int32_t));
        memcpy(&o->data_int[i], &data->data_int, sizeof(int32_t));
        break;
      case OBJ_TYPE_BYTE:
        if (i == 0)
          o->data_byte   = malloc(o->data_len * sizeof(uint8_t));
        memcpy(&o->data_byte[i], &data->data_byte, sizeof(uint8_t));
        break;
      case OBJ_TYPE_FLOAT:
        if (i == 0)
          o->data_float  = malloc(o->data_len * sizeof(float));
        memcpy(&o->data_float[i], &data->data_float, sizeof(float));
        break;
      case OBJ_TYPE_DOUBLE:
        if (i == 0)
          o->data_double = malloc(o->data_len * sizeof(double));
        memcpy(&o->data_double[i], &data->data_double, sizeof(double));
        break;
    }

    free(n->data);
    n->data = NULL;
    i++;
    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // destroy our temp list
  if (o->obj_list != NULL) {
    ex_list_destroy(o->obj_list);
    o->obj_list = NULL;
  }

  o->built = 1;
}

ex_octree_t* ex_octree_reset(ex_octree_t *o)
{
  if (o == NULL)
    return NULL;

  for (int i=0; i<8; i++)
    if (o->children[i] != NULL)
      ex_octree_reset(o->children[i]);

  if (o->obj_list != NULL) {
    ex_list_destroy(o->obj_list);
    o->obj_list = NULL;
  }

  if (o->data_len > 0 && o->data_type != OBJ_TYPE_NULL) {
    switch (o->data_type) {
      case OBJ_TYPE_UINT:
        if (o->data_uint != NULL)
          free(o->data_uint);
        break;
      case OBJ_TYPE_INT:
        if (o->data_int != NULL)
          free(o->data_int);
        break;
      case OBJ_TYPE_BYTE:
        if (o->data_byte != NULL)
          free(o->data_byte); 
        break;
      case OBJ_TYPE_FLOAT:
        if (o->data_float != NULL)
          free(o->data_float);
        break; 
      case OBJ_TYPE_DOUBLE:
        if (o->data_double != NULL)
          free(o->data_double);
        break;
    }
  }

  if (o->rendered) {
    glDeleteVertexArrays(1, &o->vao);
    glDeleteBuffers(1, &o->vbo);
    glDeleteBuffers(1, &o->ebo);
  }

  int data_type = o->data_type;
  if (!o->first) {
    free(o);
  } else {
    free(o);
    return ex_octree_new(data_type);
  }

  return NULL;
}

void ex_octree_get_colliding_count(ex_octree_t *o, rect_t *bounds, int *count)
{
  if (o == NULL)
    return;

  // add our data to the list
  void *oct_data = ex_octree_data_ptr(o);
  if (oct_data != NULL) {
    if (!ex_aabb_aabb(o->region, *bounds))
      return;

    (*count)++;
  }

  // recurse adding data to the list
  for (int i=0; i<8; i++)
    if (o->children[i] != NULL)
      ex_octree_get_colliding_count(o->children[i], bounds, count);
}

void ex_octree_get_colliding(ex_octree_t *o, rect_t *bounds, ex_octree_data_t *data_list, int *index)
{
  if (o == NULL)
    return;

  // add our data to the list
  void *oct_data = ex_octree_data_ptr(o);
  if (oct_data != NULL) {
    if (!ex_aabb_aabb(o->region, *bounds))
      return;

    data_list[*index].len = o->data_len;
    data_list[*index].data = oct_data;
    (*index)++;
  }

  // recurse adding data to the list
  for (int i=0; i<8; i++)
    if (o->children[i] != NULL)
      ex_octree_get_colliding(o->children[i], bounds, data_list, index);
}

void ex_octree_render(ex_octree_t *o)
{
  if (o == NULL || !o->built)
    return;

  int alive = 0;
  for (int i=0; i<8; i++) {
    if (o->children[i] != NULL) {
      ex_octree_render(o->children[i]);
      alive++;
    }
  }

  if (alive < 1 && o->data_len == 0)
    return;

  if (!o->rendered) {
    float vertices[EX_VERTICES_CUBE_LEN];
    memcpy(vertices, ex_vertices_cube, sizeof(float)*EX_VERTICES_CUBE_LEN);

    for (int i=0; i<EX_VERTICES_CUBE_LEN; i+=3) {
      vertices[i+0] > 0.0f ? (vertices[i+0] = o->region.max[0]) : (vertices[i+0] = o->region.min[0]);
      vertices[i+1] > 0.0f ? (vertices[i+1] = o->region.max[1]) : (vertices[i+1] = o->region.min[1]);
      vertices[i+2] > 0.0f ? (vertices[i+2] = o->region.max[2]) : (vertices[i+2] = o->region.min[2]);
    }

    glGenVertexArrays(1, &o->vao);
    glGenBuffers(1, &o->vbo);
    glGenBuffers(1, &o->ebo);
    glBindVertexArray(o->vao);

    glBindBuffer(GL_ARRAY_BUFFER, o->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*EX_VERTICES_CUBE_LEN, &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*EX_INDICES_CUBE_LEN, &ex_indices_cube[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (GLvoid*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    o->rendered = 1;
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glCullFace(GL_NONE);
  glBindVertexArray(o->vao);
  glLineWidth(0.5f);
  glDrawElements(GL_LINES, EX_INDICES_CUBE_LEN, GL_UNSIGNED_INT, 0); 
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glBindVertexArray(0);
}