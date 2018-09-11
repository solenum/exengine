#include "cache.h"
#include "scene.h"
#include "exe_list.h"

list_t *texture_list, *model_list;

void ex_cache_init()
{
  // init lists
  texture_list = list_new();
  model_list   = list_new();
}

void ex_cache_model(ex_model_t *model)
{
  printf("Caching model %s\n", model->path);

  list_add(model_list, (void*)model);
}

ex_model_t* ex_cache_get_model(const char *path)
{
  // check of model already exists
  list_node_t *n = model_list;
  while (n->data != NULL) {
    ex_model_t *m = n->data;

    // compare file paths
    if (strcmp(path, m->path) == 0) {
      // exists, return it
      printf("Returning instance of model from cache for %s\n", path);
      return ex_model_copy(m);
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // doesnt exist
  return NULL;
}

GLuint ex_cache_texture(const char *path)
{
  // check if texture already exists
  list_node_t *n = texture_list;
  while (n->data != NULL) {
    ex_texture_t *t = n->data;

    // compare file names
    if (strcmp(path, t->name) == 0) {
      // yep, return that one
      return t->id;
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  printf("Caching texture %s\n", path);

  // doesnt exist, create texture
  ex_texture_t *t = ex_texture_load(path, 0);
  if (t != NULL) {
    // store it in the list
    list_add(texture_list, (void*)t);
    return t->id;
  }

  return 0;
}

void ex_cache_flush()
{
  // cleanup textures
  list_node_t *n = texture_list;
  while (n->data != NULL) {
    ex_texture_t *t = n->data;
    
    // free texture data
    glDeleteTextures(1, &t->id);
    free(t);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free texture list
  list_destroy(texture_list);

  // cleanup models
  n = model_list;
  while (n->data != NULL) {
    ex_model_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free model list
  list_destroy(model_list);

  // re-init the cache
  ex_cache_init();
}