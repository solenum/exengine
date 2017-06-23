#ifndef MESH_H
#define MESH_H

#include "mathlib.h"

#define GLEW_STATIC
#include <GL/glew.h>

typedef struct {
  float position[3];
  float uv[2];
  float normal[3];
  float tangent[4];
  uint8_t color[4];
  uint8_t blend_indexes[4];
  uint8_t blend_weights[4];
} vertex_t;

typedef struct {
  GLuint VAO, VBO, EBO, vcount, icount;
  GLuint texture, texture_spec, texture_norm;
  mat4x4 transform;
  uint32_t current_frame;
  int use_transform;

  vec3 position, rotation;
  float scale;
  uint8_t is_lit;
} mesh_t;

/**
 * [mesh_new generate a mesh with the given vertices, indices, and texture]
 * @param  vertices [pointer to vertices]
 * @param  vcount   [length of vertices]
 * @param  indices  [pointer to indces]
 * @param  icount   [length of indices]
 * @param  texture  [the texture's uint]
 * @return          [mesh_t pointer]
 */
mesh_t* mesh_new(vertex_t *vertices, size_t vcount, GLuint *indices, size_t icount, GLuint texture);

/**
 * [mesh_draw renders a mesh to the screen]
 * @param m              [mesh_t pointer]
 * @param shader_program [shader program to use]
 */
void mesh_draw(mesh_t* m, GLuint shader_program);

/**
 * [mesh_destroy free any malloc'd data]
 * @param m [mesh_t pointer]
 */
void mesh_destroy(mesh_t *m);

#endif // MESH_H