/* mesh
  A generic mesh container.
  
  Each mesh has a single set of
  textures applied to it, models
  contain a mesh per texture set used.

  This is automatically handled by the IQM
  loader, only use the following if you
  want to manually define vertices
  via code.
*/

#ifndef EX_MESH_H
#define EX_MESH_H

#include <string.h>

#include "math/mathlib.h"
#include "glad/glad.h"

typedef struct {
  float position[3];
  float uv[2];
  float normal[3];
  float tangent[4];
  uint8_t color[4];
  uint8_t blend_indexes[4];
  uint8_t blend_weights[4];
} ex_vertex_t;

typedef struct {
  GLuint VAO, VBO, EBO, vcount, icount;
  GLuint texture, texture_spec, texture_norm;
  uint32_t current_frame;
} ex_mesh_t;

/**
 * [ex_mesh_new generate a mesh with the given vertices, indices, and texture]
 * @param  vertices [pointer to vertices]
 * @param  vcount   [length of vertices]
 * @param  indices  [pointer to indces]
 * @param  icount   [length of indices]
 * @param  texture  [the texture's uint]
 * @return          [ex_mesh_t pointer]
 */
ex_mesh_t* ex_mesh_new(ex_vertex_t *vertices, size_t vcount, GLuint *indices, size_t icount, GLuint texture);

/**
 * [ex_mesh_copy duplicates a mesh]
 * @param  mesh [the mesh to copy]
 * @return      [the new copy]
 */
ex_mesh_t* ex_mesh_copy(ex_mesh_t *mesh);

/**
 * [ex_mesh_destroy free any malloc'd data]
 * @param m [ex_mesh_t pointer]
 */
void ex_mesh_destroy(ex_mesh_t *m);

#endif // EX_MESH_H