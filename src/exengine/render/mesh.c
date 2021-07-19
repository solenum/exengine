#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render/mesh.h"
#include "render/shader.h"
#include "render/renderer.h"

ex_mesh_t* ex_mesh_new(ex_vertex_t* vertices, size_t vcount, GLuint *indices, size_t icount, GLuint texture)
{
  ex_mesh_t* m = malloc(sizeof(ex_mesh_t));

  m->texture = texture;
  m->texture_spec = 0;
  m->texture_norm = 0;
  m->vcount  = vcount;
  m->icount  = icount;

  glGenVertexArrays(1, &m->VAO);
  glGenBuffers(1, &m->VBO);
  glGenBuffers(1, &m->EBO);

  glBindVertexArray(m->VAO);

  // vertices
  glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ex_vertex_t)*m->vcount, &vertices[0], GL_STATIC_DRAW);

  // indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*m->icount, &indices[0], GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // tex coords
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // normals
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(5 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  // tangents
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(8 * sizeof(GLfloat)));
  glEnableVertexAttribArray(3);

  // color
  glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)));
  glEnableVertexAttribArray(4);

  // blend indexes
  glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)+(4 * sizeof(GLubyte))));
  glEnableVertexAttribArray(5);

  // blend weights
  glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)+(8 * sizeof(GLubyte))));
  glEnableVertexAttribArray(6);

  glBindVertexArray(0);

  return m;
}

ex_mesh_t* ex_mesh_copy(ex_mesh_t *mesh)
{
  ex_mesh_t *m = malloc(sizeof(ex_mesh_t));

  m->VBO = mesh->VBO;
  m->EBO = mesh->EBO;
  m->texture = mesh->texture;
  m->vcount  = mesh->vcount;
  m->icount  = mesh->icount;
  m->texture_spec = mesh->texture_spec;
  m->texture_norm = mesh->texture_norm;

  glGenVertexArrays(1, &m->VAO);
  glBindVertexArray(m->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
  
    // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // tex coords
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // normals
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(5 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  // tangents
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(8 * sizeof(GLfloat)));
  glEnableVertexAttribArray(3);

  // color
  glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)));
  glEnableVertexAttribArray(4);

  // blend indexes
  glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)+(4 * sizeof(GLubyte))));
  glEnableVertexAttribArray(5);

  // blend weights
  glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)+(8 * sizeof(GLubyte))));
  glEnableVertexAttribArray(6);

  glBindVertexArray(0);

  return m;
}

void ex_mesh_destroy(ex_mesh_t* m)
{
  glDeleteVertexArrays(1, &m->VAO);
  glDeleteBuffers(1, &m->VBO);
  glDeleteBuffers(1, &m->EBO);

  free(m);
  m = NULL;
}