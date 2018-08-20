#include "mesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shader.h"
#include "defaults.h"

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

void ex_mesh_draw(ex_mesh_t* m, GLuint shader_program, int count)
{
  // bind vao/ebo/tex
  glBindVertexArray(m->VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
  glUniform1i(ex_uniform(shader_program, "u_texture"), 4);
  glUniform1i(ex_uniform(shader_program, "u_spec"), 5);
  glUniform1i(ex_uniform(shader_program, "u_norm"), 6);

  // diffuse  
  glActiveTexture(GL_TEXTURE4);
  if (m->texture < 1)
    glBindTexture(GL_TEXTURE_2D, default_texture_diffuse);
  else
    glBindTexture(GL_TEXTURE_2D, m->texture);

  // specular
  glActiveTexture(GL_TEXTURE5);
  if (m->texture_spec < 1)
    glBindTexture(GL_TEXTURE_2D, default_texture_specular);
  else
    glBindTexture(GL_TEXTURE_2D, m->texture_spec);

  // normal
  glActiveTexture(GL_TEXTURE6);
  if (m->texture_norm < 1)
    glBindTexture(GL_TEXTURE_2D, default_texture_normal);
  else
    glBindTexture(GL_TEXTURE_2D, m->texture_norm);

  // draw mesh
  glDrawElementsInstanced(GL_TRIANGLES, m->icount, GL_UNSIGNED_INT, 0, count);

  // unbind buffers
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void ex_mesh_destroy(ex_mesh_t* m)
{
  glDeleteVertexArrays(1, &m->VAO);
  glDeleteBuffers(1, &m->VBO);
  glDeleteBuffers(1, &m->EBO);

  free(m);
  m = NULL;
}