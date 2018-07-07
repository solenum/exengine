#include "pointlight.h"
#include "shader.h"
#include <stdlib.h>
#include <string.h>

#define SHADOW_MAP_SIZE 1024
mat4x4 point_shadow_projection;
GLuint point_light_shader;

void ex_point_light_init()
{
  // compile the shaders
  point_light_shader = ex_shader_compile("pointfbo.vs", "pointfbo.fs", "pointfbo.gs");

  float aspect = (float)SHADOW_MAP_SIZE/(float)SHADOW_MAP_SIZE;
  mat4x4_perspective(point_shadow_projection, rad(90.0f), aspect, 0.1f, EX_POINT_FAR_PLANE); 
}

ex_point_light_t *ex_point_light_new(vec3 pos, vec3 color, int dynamic)
{
  ex_point_light_t *l = malloc(sizeof(ex_point_light_t));

  // set light properties
  memcpy(l->position, pos, sizeof(vec3));
  memcpy(l->color, color, sizeof(vec3));

  // generate cube map
  glGenTextures(1, &l->depth_map);
  glBindTexture(GL_TEXTURE_CUBE_MAP, l->depth_map);
  for (int i=0; i<6; i++)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16,
      SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border);  

  // only want the depth buffer
  glGenFramebuffers(1, &l->depth_map_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, l->depth_map_fbo);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, l->depth_map, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Error! Point light framebuffer is not complete!\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  l->shader     = point_light_shader;
  l->dynamic    = dynamic;
  l->update     = 1;
  l->is_shadow  = 1;
  l->is_visible = 1;

  return l;
}

void ex_point_light_begin(ex_point_light_t *l)
{
  l->update = 0;

  // dont ask
  vec3 temp;
  vec3_add(temp, l->position, (vec3){1.0f, 0.0f, 0.0f});
  mat4x4_look_at(l->transform[0], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(l->transform[0], point_shadow_projection, l->transform[0]);

  vec3_add(temp, l->position, (vec3){-1.0f, 0.0f, 0.0f});
  mat4x4_look_at(l->transform[1], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(l->transform[1], point_shadow_projection, l->transform[1]);

  vec3_add(temp, l->position, (vec3){0.0f, 1.0f, 0.0f});
  mat4x4_look_at(l->transform[2], l->position, temp, (vec3){0.0f, 0.0f, 1.0f});
  mat4x4_mul(l->transform[2], point_shadow_projection, l->transform[2]);

  vec3_add(temp, l->position, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_look_at(l->transform[3], l->position, temp, (vec3){0.0f, 0.0f, -1.0f});
  mat4x4_mul(l->transform[3], point_shadow_projection, l->transform[3]);

  vec3_add(temp, l->position, (vec3){0.0f, 0.0f, 1.0f});
  mat4x4_look_at(l->transform[4], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(l->transform[4], point_shadow_projection, l->transform[4]);
 
  vec3_add(temp, l->position, (vec3){0.0f, 0.0f, -1.0f});
  mat4x4_look_at(l->transform[5], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
  mat4x4_mul(l->transform[5], point_shadow_projection, l->transform[5]);

  // render to depth cube map
  glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
  glBindFramebuffer(GL_FRAMEBUFFER, l->depth_map_fbo);

  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glUseProgram(l->shader);

  // pass transform matrices to shader
  glUniformMatrix4fv(ex_uniform(l->shader, "u_shadow_matrices[0]"), 1, GL_FALSE, *l->transform[0]);
  glUniformMatrix4fv(ex_uniform(l->shader, "u_shadow_matrices[1]"), 1, GL_FALSE, *l->transform[1]);
  glUniformMatrix4fv(ex_uniform(l->shader, "u_shadow_matrices[2]"), 1, GL_FALSE, *l->transform[2]);
  glUniformMatrix4fv(ex_uniform(l->shader, "u_shadow_matrices[3]"), 1, GL_FALSE, *l->transform[3]);
  glUniformMatrix4fv(ex_uniform(l->shader, "u_shadow_matrices[4]"), 1, GL_FALSE, *l->transform[4]);
  glUniformMatrix4fv(ex_uniform(l->shader, "u_shadow_matrices[5]"), 1, GL_FALSE, *l->transform[5]);

  glUniform1f(ex_uniform(l->shader, "u_far_plane"), EX_POINT_FAR_PLANE);
  glUniform3fv(ex_uniform(l->shader, "u_light_pos"), 1, l->position);
}

void ex_point_light_draw(ex_point_light_t *l, GLuint shader, const char *prefix)
{
  if (l->is_shadow) {
    glUniform1i(ex_uniform(shader, "u_point_light.is_shadow"), 1);
    
    glUniform1i(ex_uniform(shader, "u_point_depth"), 4);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, l->depth_map);
  } else if (prefix != NULL) {
    char buff[64];
    sprintf(buff, "%s.is_shadow", prefix);
    glUniform1i(ex_uniform(shader, buff), 0);
  }

  if (prefix != NULL) {
    char buff[64];
    sprintf(buff, "%s.far", prefix);
    glUniform1f(ex_uniform(shader,  buff), EX_POINT_FAR_PLANE);
    sprintf(buff, "%s.position", prefix);
    glUniform3fv(ex_uniform(shader, buff), 1, l->position);
    sprintf(buff, "%s.color", prefix);
    glUniform3fv(ex_uniform(shader, buff), 1, l->color);
  } else {
    glUniform1i(ex_uniform(shader,  "u_point_active"), 1);
    glUniform1f(ex_uniform(shader,  "u_point_light.far"), EX_POINT_FAR_PLANE);
    glUniform3fv(ex_uniform(shader, "u_point_light.position"), 1, l->position);
    glUniform3fv(ex_uniform(shader, "u_point_light.color"), 1, l->color);
  }
}

void ex_point_light_destroy(ex_point_light_t *l)
{
  glDeleteFramebuffers(1, &l->depth_map_fbo);
  glDeleteTextures(1, &l->depth_map);
  free(l);
}