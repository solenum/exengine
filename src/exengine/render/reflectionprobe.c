#include <stdlib.h>
#include <string.h>
#include "render/reflectionprobe.h"
#include "render/shader.h"

GLuint reflection_shader;
mat4x4 reflection_projection;

#define REFLECTION_RESOLUTION 1024
#define RELFECTION_FAR 75

void ex_reflection_init()
{
  reflection_shader = ex_shader("reflection.glsl");

  float aspect = (float)REFLECTION_RESOLUTION/(float)REFLECTION_RESOLUTION;
  mat4x4_perspective(reflection_projection, rad(90.0f), aspect, 0.1f, RELFECTION_FAR);
}

ex_reflection_t *ex_reflection_new(vec3 position)
{
  ex_reflection_t *r = malloc(sizeof(ex_reflection_t));

  // set properties
  memcpy(r->position, position, sizeof(vec3));

  glGenFramebuffers(1, &r->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, r->fbo);

  // generate cube map
  glGenTextures(1, &r->color_buffer);
  glBindTexture(GL_TEXTURE_CUBE_MAP, r->color_buffer);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  for (int i=0; i<6; i++)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16,
      REFLECTION_RESOLUTION, REFLECTION_RESOLUTION, 0, GL_RGB, GL_FLOAT, NULL);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, r->color_buffer, 0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);

  glGenTextures(1, &r->depth_buffer);
  glBindTexture(GL_TEXTURE_CUBE_MAP, r->depth_buffer);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  for (int i=0; i<6; i++)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16, REFLECTION_RESOLUTION, REFLECTION_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, r->depth_buffer, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Error! Reflection framebuffer is not complete!\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  r->update = 1;
  r->shader = reflection_shader;

  return r;
}

void ex_reflection_begin(ex_reflection_t *r)
{
  r->update = 0;

  vec3 temp;
  vec3_add(temp, r->position, (vec3){1.0f, 0.0f, 0.0f});
  mat4x4_look_at(r->transform[0], r->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(r->transform[0], reflection_projection, r->transform[0]);

  vec3_add(temp, r->position, (vec3){-1.0f, 0.0f, 0.0f});
  mat4x4_look_at(r->transform[1], r->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(r->transform[1], reflection_projection, r->transform[1]);

  vec3_add(temp, r->position, (vec3){0.0f, 1.0f, 0.0f});
  mat4x4_look_at(r->transform[2], r->position, temp, (vec3){0.0f, 0.0f, 1.0f});
  mat4x4_mul(r->transform[2], reflection_projection, r->transform[2]);

  vec3_add(temp, r->position, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_look_at(r->transform[3], r->position, temp, (vec3){0.0f, 0.0f, -1.0f});
  mat4x4_mul(r->transform[3], reflection_projection, r->transform[3]);

  vec3_add(temp, r->position, (vec3){0.0f, 0.0f, 1.0f});
  mat4x4_look_at(r->transform[4], r->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(r->transform[4], reflection_projection, r->transform[4]);
 
  vec3_add(temp, r->position, (vec3){0.0f, 0.0f, -1.0f});
  mat4x4_look_at(r->transform[5], r->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(r->transform[5], reflection_projection, r->transform[5]);

  // render to depth cube map
  glUseProgram(reflection_shader);
  glViewport(0, 0, REFLECTION_RESOLUTION, REFLECTION_RESOLUTION);
  glBindFramebuffer(GL_FRAMEBUFFER, r->fbo);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);

  // pass transform matrices to shader
  glUniformMatrix4fv(ex_uniform(reflection_shader, "u_shadow_matrices[0]"), 1, GL_FALSE, *r->transform[0]);
  glUniformMatrix4fv(ex_uniform(reflection_shader, "u_shadow_matrices[1]"), 1, GL_FALSE, *r->transform[1]);
  glUniformMatrix4fv(ex_uniform(reflection_shader, "u_shadow_matrices[2]"), 1, GL_FALSE, *r->transform[2]);
  glUniformMatrix4fv(ex_uniform(reflection_shader, "u_shadow_matrices[3]"), 1, GL_FALSE, *r->transform[3]);
  glUniformMatrix4fv(ex_uniform(reflection_shader, "u_shadow_matrices[4]"), 1, GL_FALSE, *r->transform[4]);
  glUniformMatrix4fv(ex_uniform(reflection_shader, "u_shadow_matrices[5]"), 1, GL_FALSE, *r->transform[5]);
  glUniform3fv(ex_uniform(reflection_shader, "u_probe_pos"), 1, &r->position[0]);
}

void ex_reflection_draw(ex_reflection_t *r, GLuint shader)
{
  glUniform1i(ex_uniform(shader, "u_reflection"), 7);
  
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_CUBE_MAP, r->color_buffer);
}

void ex_reflection_destroy(ex_reflection_t *r)
{
  glDeleteFramebuffers(1, &r->fbo);
  glDeleteTextures(1, &r->color_buffer);
  glDeleteTextures(1, &r->depth_buffer);
  free(r);
}