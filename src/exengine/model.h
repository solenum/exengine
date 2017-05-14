#ifndef MODEL_H
#define MODEL_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "exe_list.h"
#include "mesh.h"

typedef struct {
  list_t *mesh_list;

  vec3 position, rotation;
  float scale;
  uint8_t is_lit;
} model_t;

model_t* model_new();

void model_update(model_t *m, float delta_time);

void model_draw(model_t *m, GLuint shader);

void model_destroy(model_t *m);

#endif // MODEL_H