#ifndef OCTREE_H
#define OCTREE_H

#include <stdlib.h>
#include <inttypes.h>
#include "mathlib.h"

#define OCT_MIN_SIZE 5

typedef struct octree_t octree_t;

struct octree_t {
  vec3 region[8];
  octree_t *children[8];
  octree_t *parent;
  int max_life, cur_life;
  uint8_t ready, built, active;
  vec3 *data;
  size_t data_len;
};

octree_t* octree_new();

/*int octree_get_containing_point(octree_t *o, vec3 point);

void octree_insert(octree_t *o, vec3 point);

void octree_get_points(octree_t *o, vec3 min, vec3 max, octree_points_t *points, vec3 *data);

static inline size_t octree_get_len(octree_t *o, size_t *len) {
  for (int i=0; i<8; i++) { 
    if (o->children[i] != NULL) {
      len++;
      octree_get_len(o->children[i], len);
    }
  }
} */

#endif // OCTREE_H