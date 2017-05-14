#include <string.h>
#include "model.h"

model_t* model_new()
{
  // init lists etc
  model_t *m = malloc(sizeof(model_t));
  m->mesh_list = list_new();

  // init attributes
  memset(m->position, 0, sizeof(vec3));
  memset(m->rotation, 0, sizeof(vec3));
  m->scale  = 1.0f;
  m->is_lit = 1;

  return m;
}

void model_update(model_t *m, float delta_time)
{
  // render meshes
  list_node_t *n = m->mesh_list;
  while (n->data != NULL) {
    // update attributes
    mesh_t *mesh = n->data;
    memcpy(mesh->position, m->position, sizeof(vec3));
    memcpy(mesh->rotation, m->rotation, sizeof(vec3));
    mesh->scale  = m->scale;
    mesh->is_lit = m->is_lit; 

    // update mesh
    mesh_update(mesh, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

void model_draw(model_t *m, GLuint shader)
{
  // render meshes
  list_node_t *n = m->mesh_list;
  while (n->data != NULL) {
    mesh_draw(n->data, shader);
    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

void model_destroy(model_t *m)
{
  // cleanup meshes
  list_node_t *n = m->mesh_list;
  while (n->data != NULL) {
    mesh_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free mesh list
  list_destroy(m->mesh_list);

  // free model
  free(m);
}