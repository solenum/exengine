#include "mesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ex_mesh_t* ex_mesh_new(ex_vertex_t* vertices, size_t vcount, GLuint *indices, size_t icount, GLuint texture)
{
  ex_mesh_t* m = malloc(sizeof(ex_mesh_t));

  m->texture = texture;
  m->texture_spec = 0;
  m->texture_norm = 0;
  m->vcount  = vcount;
  m->icount  = icount;
  m->is_lit  = 1;
  m->scale   = 1.0f;

  memset(m->position, 0, sizeof(vec3));
  memset(m->rotation, 0, sizeof(vec3));

  mat4x4_identity(m->transform);

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

void ex_mesh_draw(ex_mesh_t* m, GLuint shader_program)
{
  // handle transformations
  if (!m->use_transform) {
    mat4x4_identity(m->transform);
    mat4x4_translate_in_place(m->transform, m->position[0], m->position[1], m->position[2]);
    mat4x4_rotate_Y(m->transform, m->transform, rad(m->rotation[1]));
    mat4x4_rotate_X(m->transform, m->transform, rad(m->rotation[0]));
    mat4x4_rotate_Z(m->transform, m->transform, rad(m->rotation[2]));
    mat4x4_scale_aniso(m->transform, m->transform, m->scale, m->scale, m->scale);
  }

  // bind vao/ebo/tex
  glBindVertexArray(m->VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
  glUniform1i(glGetUniformLocation(shader_program, "u_texture"), 4);
  glUniform1i(glGetUniformLocation(shader_program, "u_spec"), 5);
  glUniform1i(glGetUniformLocation(shader_program, "u_norm"), 6);
  
  GLuint is_lit_loc = glGetUniformLocation(shader_program, "u_is_lit");
  glUniform1i(is_lit_loc, m->is_lit);

  GLuint is_texture_loc = glGetUniformLocation(shader_program, "u_is_textured");
  GLuint is_spec_loc = glGetUniformLocation(shader_program, "u_is_spec");
  GLuint is_norm_loc = glGetUniformLocation(shader_program, "u_is_norm");
  
  if (m->texture < 1) {
    glUniform1i(is_texture_loc, 0);
  } else { 
    glUniform1i(is_texture_loc, 1);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m->texture);
  }

  if (m->texture_spec < 1) {
    glUniform1i(is_spec_loc, 0);
  } else {
    glUniform1i(is_spec_loc, 1);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m->texture_spec);
  }

  if (m->texture_norm < 1) {
    glUniform1i(is_norm_loc, 0);
  } else {
    glUniform1i(is_norm_loc, 1);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m->texture_norm);
  }

  // pass transform matrix to shader
  GLuint transform_loc = glGetUniformLocation(shader_program, "u_model");
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, m->transform[0]);

  // draw mesh
  glDrawElements(GL_TRIANGLES, m->icount, GL_UNSIGNED_INT, 0);

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