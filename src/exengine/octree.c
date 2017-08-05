#include "octree.h"
#include <stdio.h>

octree_t* octree_new()
{
  octree_t *o = malloc(sizeof(octree_t));

  for (int i=0; i<8; i++) {
    o->children[i] = NULL;
  }
  memset(o->region.min, 0, sizeof(vec3));
  memset(o->region.max, 0, sizeof(vec3));

  o->ready     = 0;
  o->built     = 0;
  o->max_life  = 8;
  o->cur_life  =-1;
  o->active    = 0;
  o->obj_list  = NULL;
  o->parent    = NULL;
  o->obj_list  = list_new();

  return o;
}

void octree_init(octree_t *o, rect_t region, list_t *objects)
{
  memcpy(&o->region, &region, sizeof(region));
  o->obj_list = objects;
  o->active   = 0;
  o->max_life = 8;
  o->cur_life = -1;
  o->ready    = 0;
  o->built    = 0;
}

void octree_build(octree_t *o)
{
  if (o->obj_list->data == NULL)
    return;

  // our size
  vec3 region;
  vec3_sub(region, o->region.max, o->region.min);

  if (region[0] <= OCT_MIN_SIZE && region[1] <= OCT_MIN_SIZE && region[2] <= OCT_MIN_SIZE)
    return;

  vec3 half, center;
  vec3_scale(half, region, 0.5f);
  vec3_add(center, o->region.min, half);

  // octant regions
  rect_t octants[8];
  octants[0] = rect_new(o->region.min, center);
  octants[1] = rect_new((vec3){center[0], o->region.min[1], o->region.min[2]}, (vec3){o->region.max[0], center[1], center[2]});
  octants[2] = rect_new((vec3){center[0], o->region.min[1], center[2]}, (vec3){o->region.max[0], center[1], o->region.max[2]});
  octants[3] = rect_new((vec3){o->region.min[0], o->region.min[1], center[2]}, (vec3){center[0], center[1], o->region.max[2]});
  octants[4] = rect_new((vec3){o->region.min[0], center[1], o->region.min[2]}, (vec3){center[0], o->region.max[1], center[2]});
  octants[5] = rect_new((vec3){center[0], center[1], o->region.min[2]}, (vec3){o->region.max[0], o->region.max[1], center[2]});
  octants[6] = rect_new(center, o->region.max);
  octants[7] = rect_new((vec3){o->region.min[0], center[1], center[2]}, (vec3){center[0], o->region.max[1], o->region.max[2]});

  // object lists
  list_t *obj_lists[8];
  for (int i=0; i<8; i++)
    obj_lists[i] = list_new();

  // add objects to appropriate octant
  int i = 0;
  list_node_t *n = o->obj_list;
  while (n->data != NULL) {
    int found = 0;

    for (int j=0; j<8; j++) {
      octree_obj_t *obj = n->data;
      if (aabb_aabb(octants[j], obj->box)) {
        list_add(obj_lists[j], (void*)n->data);
        found = 1;
        break;
      }
    }

    // remove obj from this list
    i++;
    if (found) {
      list_t *next = n->next;
      list_remove(n, n->data);
      if (next != NULL) {
        n = next;
        continue;
      } else {
        break;
      }
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // create children
  for (int i=0; i<8; i++) {
    if (obj_lists[i]->data != NULL) {
      o->children[i] = malloc(sizeof(octree_t));
      octree_init(o->children[i], octants[i], obj_lists[i]);
      o->active |= (1<<i);
      octree_build(o->children[i]);
    }
  }

  o->built = 1;
  o->ready = 1;
}