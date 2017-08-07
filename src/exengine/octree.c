#include "octree.h"
#include "vertices.h"
#include <stdio.h>

octree_t* octree_new()
{
  octree_t *o = malloc(sizeof(octree_t));

  for (int i=0; i<8; i++) {
    o->children[i] = NULL;
  }
  memset(o->region.min, 0.0f, sizeof(vec3));
  memset(o->region.max, 1.0f, sizeof(vec3));

  o->rendered = 0;
  o->built    = 0;
  o->obj_list = list_new();

  return o;
}

void octree_init(octree_t *o, rect_t region, list_t *objects)
{
  memcpy(&o->region, &region, sizeof(region));
  for (int i=0; i<8; i++) {
    o->children[i] = NULL;
  }
  o->obj_list = objects;
  o->rendered = 0;
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
        // obj should be added to node its most in by % ?
        // break here to insert into one only?
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
      octree_build(o->children[i]);
    }
  }

  o->built = 1;
}

void octree_reset(octree_t *o)
{
  for (int i=0; i<8; i++) {
    if (o->children[i])
      octree_reset(o->children[i]);

    if (o->children[i]->obj_list)
      list_destroy(o->children[i]->obj_list);

    if (o->children[i]->rendered) {
      glDeleteVertexArrays(1, &o->children[i]->vao);
      glDeleteBuffers(1, &o->children[i]->vbo);
      o->children[i]->rendered = 0;
    }

    if (o->children[i])
      free(o->children[i]);
  
    if (o->rendered) {
      glDeleteVertexArrays(1, &o->vao);
      glDeleteBuffers(1, &o->vbo);
      o->rendered = 0;
    }
  }
}

octree_t* octree_coll_objects(octree_t *o, rect_t *bounds)
{
  if (!aabb_aabb(o->region, *bounds))
    return NULL;

  for (int i=0; i<8; i++) {
    if (o->children[i] != NULL && aabb_aabb(o->children[i]->region, *bounds))
      return octree_coll_objects(o->children[i], bounds);
  }

  return o;
}

void octree_render(octree_t *o)
{
  if (o == NULL || o->obj_list->data == NULL)
    return;

  if (!o->rendered) {
    float vertices[VERTICES_CUBE_LEN];
    memcpy(vertices, vertices_cube, sizeof(float)*VERTICES_CUBE_LEN);

    for (int i=0; i<VERTICES_CUBE_LEN; i+=3) {
      vertices[i+0] > 0.0f ? (vertices[i+0] = o->region.max[0]) : (vertices[i+0] = o->region.min[0]);
      vertices[i+1] > 0.0f ? (vertices[i+1] = o->region.max[1]) : (vertices[i+1] = o->region.min[1]);
      vertices[i+2] > 0.0f ? (vertices[i+2] = o->region.max[2]) : (vertices[i+2] = o->region.min[2]);
    }

    glGenVertexArrays(1, &o->vao);
    glGenBuffers(1, &o->vbo);
    glGenBuffers(1, &o->ebo);
    glBindVertexArray(o->vao);

    glBindBuffer(GL_ARRAY_BUFFER, o->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*VERTICES_CUBE_LEN, &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*INDICES_CUBE_LEN, &indices_cube[0], GL_STATIC_DRAW);

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
  glLineWidth(1.5f);
  glDrawElements(GL_LINES, INDICES_CUBE_LEN, GL_UNSIGNED_INT, 0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glBindVertexArray(0);

  for (int i=0; i<8; i++)
    if (o->children[i] != NULL)
      octree_render(o->children[i]);
}