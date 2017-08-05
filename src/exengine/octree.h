#ifndef OCTREE_H
#define OCTREE_H

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "exe_list.h"
#include "mathlib.h" 

#define OCT_MIN_SIZE 1

typedef struct {
  vec3 min, max;
} rect_t;

typedef struct {
  vec3 a,b,c;
  rect_t box;
} octree_obj_t;

typedef struct octree_t octree_t;
struct octree_t {
  rect_t region;
  octree_t *children[8];
  octree_t *parent;
  int max_life, cur_life;
  list_t *obj_list;
  // flags etc
  uint8_t ready  : 1;
  uint8_t built  : 1;
  uint8_t active;
};

octree_t* octree_new();

void octree_init(octree_t *o, rect_t region, list_t *objects);

void octree_build(octree_t *o);

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
    return a.min[0] <= b.max[0] &&
    a.max[0] >= b.min[0] &&
    a.min[1] <= b.max[1] &&
    a.max[1] >= b.min[1] &&
    a.min[2] <= b.max[2] &&
    a.max[2] >= b.min[2];
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