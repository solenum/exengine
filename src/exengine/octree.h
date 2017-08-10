#ifndef OCTREE_H
#define OCTREE_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "exe_list.h"
#include "mathlib.h" 

#define OCTREE_DEFAULT_MIN_SIZE 5.0f
extern int octree_min_size;

enum {
  OBJ_TYPE_UINT,
  OBJ_TYPE_INT,
  OBJ_TYPE_BYTE,
  OBJ_TYPE_FLOAT,
  OBJ_TYPE_DOUBLE,
  OBJ_TYPE_NULL
} octree_obj_type;

typedef struct {
  vec3 min, max;
} rect_t;

typedef struct {
  union {
    uint32_t data_uint;
    int32_t  data_int;
    uint8_t  data_byte;
    float    data_float;
    double   data_double;
  };
  rect_t box;
} octree_obj_t;

typedef struct {
  void *data;
  size_t len;
} octree_data_t;

typedef struct octree_t octree_t;
struct octree_t {
  rect_t region;
  octree_t *children[8];
  int max_life, cur_life;
  list_t *obj_list;
  // flags etc
  uint8_t rendered  : 1;
  uint8_t built     : 1;
  uint8_t first     : 1;
  uint8_t data_type : 5;
  // data
  size_t  data_len;
  union {
    uint32_t *data_uint;
    int32_t  *data_int;
    uint8_t  *data_byte;
    float    *data_float;
    double   *data_double;
  };
  // debug render stuffs
  GLuint vbo, vao, ebo;
};

octree_t* octree_new(uint8_t type);

void octree_init(octree_t *o, rect_t region, list_t *objects);

void octree_build(octree_t *o);

void octree_finalize(octree_t *o);

octree_t* octree_reset(octree_t *o);

void octree_get_colliding(octree_t *o, rect_t *bounds, list_t *data_list);

void octree_render(octree_t *o);

static inline void octree_clean_colliding(list_t *data) {
  while (data->data != NULL) {
    if (data->data != NULL)
      free(data->data);

    if (data->next != NULL)
      data = data->next;
    else
      break;
  }

  list_destroy(data);
}

static inline void* octree_data_ptr(octree_t *o) {
  switch (o->data_type) {
    case OBJ_TYPE_UINT:
      if (o->data_uint != NULL)
        return o->data_uint;
      break;
    case OBJ_TYPE_INT:
      if (o->data_int != NULL)
        return o->data_int;
      break;
    case OBJ_TYPE_BYTE:
      if (o->data_byte != NULL)
        return o->data_byte;
      break;
    case OBJ_TYPE_FLOAT:
      if (o->data_float != NULL)
        return o->data_float;
      break;
    case OBJ_TYPE_DOUBLE:
      if (o->data_double != NULL)
        return o->data_double;
      break;
  }
}

static inline rect_t rect_new(vec3 min, vec3 max) {
  rect_t r;
  memcpy(r.min, min, sizeof(vec3));
  memcpy(r.max, max, sizeof(vec3));
  return r;
};

static inline float squared(float v) { return v * v; };
static inline int rect_intersect_sphere(rect_t r, vec3 pos, float radius) {
  float dist = radius * radius;
  if (pos[0] < r.min[0]) dist -= squared(pos[0] - r.min[0]);
  else if (pos[0] > r.max[0]) dist -= squared(pos[0] - r.max[0]);
  if (pos[1] < r.min[1]) dist -= squared(pos[1] - r.min[1]);
  else if (pos[1] > r.max[1]) dist -= squared(pos[1] - r.max[1]);
  if (pos[2] < r.min[2]) dist -= squared(pos[2] - r.min[2]);
  else if (pos[2] > r.max[2]) dist -= squared(pos[2] - r.max[2]);
  return dist > 0;
};

static inline int aabb_aabb(rect_t a, rect_t b) {
  return (a.max[0] >= b.min[0] &&
          a.min[0] <= b.max[0] &&
          a.max[1] >= b.min[1] &&
          a.min[1] <= b.max[1] &&
          a.max[2] >= b.min[2] &&
          a.min[2] <= b.max[2]);
};

static inline int aabb_inside(rect_t outer, rect_t inner) {
  return (outer.min[0] <= inner.min[0] &&
          outer.max[0] >= inner.max[0] &&
          outer.min[1] <= inner.min[1] &&
          outer.max[1] >= inner.max[1] &&
          outer.min[2] <= inner.min[2] &&
          outer.max[2] >= inner.max[2]);
};

static inline int tri_in_rect(rect_t r, octree_obj_t *obj) {

};

static inline rect_t rect_from_triangle(vec3 tri[3]) {
  rect_t box;

  memcpy(box.min, tri[0], sizeof(vec3));
  vec3_min(box.min, box.min, tri[1]);
  vec3_min(box.min, box.min, tri[2]);

  memcpy(box.max, tri[0], sizeof(vec3));
  vec3_max(box.max, box.max, tri[1]);
  vec3_max(box.max, box.max, tri[2]);

  return box;
};


#endif // OCTREE_H