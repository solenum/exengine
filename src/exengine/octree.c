#include "octree.h"
#include <string.h>
#include <stdio.h>

octree_t* octree_new()
{
  octree_t *o = malloc(sizeof(octree_t));

  for (int i=0; i<8; i++) {
    o->children[i] = NULL;
    memset(o->region[i], 0, sizeof(vec3));
  }

  o->data_len = 0;
  o->ready    = 0;
  o->built    = 0;
  o->max_life = 8;
  o->cur_life =-1;
  o->active   = 0;
  o->data     = NULL;
  o->parent   = NULL;

  return o;
}


/*int octree_get_containing_point(octree_t *o, vec3 point)
{
  int oct = 0;
  if (point[0] >= o->origin[0]) oct |= 4;
  if (point[1] >= o->origin[1]) oct |= 2;
  if (point[2] >= o->origin[2]) oct |= 1;
  return oct;
}

void octree_insert(octree_t *o, vec3 point)
{
  // insert data into octree
  if (o->children[0] == NULL) {
    // insert here if we have no data
    if (o->has_data == 0) {
      memcpy(o->point, point, sizeof(vec3));
      o->has_data = 1;
      return;
    } else {
      // remove data
      vec3 prev_point;
      memcpy(prev_point, o->point, sizeof(vec3));
      o->has_data = 0;

      // set the children
      for (int i=0; i<8; i++) {
        // remove origin
        vec3 origin;
        memcpy(origin, o->origin, sizeof(vec3));

        // get bounds for new child
        origin[0] += o->half_size[0] * (i&4 ? 0.5f : -0.5f);
        origin[1] += o->half_size[1] * (i&2 ? 0.5f : -0.5f);
        origin[2] += o->half_size[2] * (i&1 ? 0.5f : -0.5f);
        
        // set new bounds
        o->children[i] = octree_new(origin, o->half_size);
        vec3_scale(o->children[i]->half_size, o->children[i]->half_size, 0.5f);
      }
      
      int a = octree_get_containing_point(o, prev_point);
      int b = octree_get_containing_point(o, point);
        
      printf("!%i! !%i!\n", a, b);
      // insert old and new point
      octree_insert(o->children[a], prev_point);
      octree_insert(o->children[b], point);
    }
  } else {
    // recurse insert to child node
    int octant = octree_get_containing_point(o, point);
    octree_insert(o->children[octant], point);
  }
}

void octree_get_points(octree_t *o, vec3 min, vec3 max, octree_points_t *points, vec3 *data)
{
  // first iteration, set up data array
  if (data == NULL) {
    // get octree length
    size_t len = 0;
    octree_get_len(o, &len);
    points = malloc(sizeof(octree_points_t));
    points->data = malloc(sizeof(vec3) * len);
    points->last = 0;
  }

  // push back points into data array
  if (o->children[0] == NULL) {
    if (o->has_data) {
      vec3 p;
      memcpy(p, o->point, sizeof(vec3));

      // check bounds
      if (p[0] > max[0] || p[1] > max[1] || p[2] > max[2])
        return;
      if (p[0] < min[0] || p[1] < min[1] || p[2] < min[2])
        return;

      // store it
      memcpy(points->data[points->last++], p, sizeof(vec3));
    }
  } else {
    for (int i=0; i<8; i++) {
      // get corners of octant
      vec3 cmin, cmax;
      vec3_add(cmax, o->children[i]->origin, o->children[i]->half_size);
      vec3_sub(cmin, o->children[i]->origin, o->children[i]->half_size);

      // check if outside of bounding box
      if (cmax[0] < min[0] || cmax[1] < min[1] || cmax[2] < min[2])
        continue;
      if (cmin[0] > max[0] || cmin[1] > max[1] || cmin[2] > max[2])
        continue;

      octree_get_points(o->children[i], min, max, points, points->data);
    }
  }
} */