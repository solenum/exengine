#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "render/spotlight.h"
#include "render/dirlight.h"
#include "render/shader.h"

#define SHADOW_MAP_SIZE 1024

mat4x4 spot_shadow_projection;
GLuint spot_light_shader;

void ex_spot_light_init()
{
  spot_light_shader = ex_dir_light_shader;

  float aspect = (float)SHADOW_MAP_SIZE/(float)SHADOW_MAP_SIZE;
  mat4x4_perspective(spot_shadow_projection, rad(90.0f), aspect, 0.1f, EX_SPOT_FAR_PLANE); 
}

ex_spot_light_t* ex_spot_light_new(vec3 pos, vec3 color, int dynamic)
{
  ex_spot_light_t *l = malloc(sizeof(ex_spot_light_t));

  memcpy(l->position, pos, sizeof(vec3));
  memcpy(l->color, color, sizeof(vec3));
  memset(l->direction, 0, sizeof(vec3));

  // generate depth map
  glGenTextures(1, &l->depth_map);
  glBindTexture(GL_TEXTURE_2D, l->depth_map);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
    SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

  glGenFramebuffers(1, &l->depth_map_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, l->depth_map_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, l->depth_map, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Error! Spot light framebuffer is not complete!\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  l->shader  = spot_light_shader;
  l->dynamic = dynamic;
  l->update  = 1;
  l->inner   = rad(12.5f);
  l->outer   = rad(16.5f);
  l->is_shadow  = 1;
  l->is_visible = 1;

  return l;
}

void ex_spot_light_begin(ex_spot_light_t *l)
{
  l->update = 0;

  // setup projection
  vec3 target;
  vec3_add(target, l->position, l->direction);
  mat4x4_look_at(l->transform, l->position, target, (vec3){0.0f, 1.0f, 0.0f});
  mat4x4_mul(l->transform, spot_shadow_projection, l->transform);

  glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
  glBindFramebuffer(GL_FRAMEBUFFER, l->depth_map_fbo);

  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glUseProgram(l->shader);

  glUniformMatrix4fv(ex_uniform(l->shader, "u_light_transform"), 1, GL_FALSE, &l->transform[0][0]);
}

void ex_spot_light_draw(ex_spot_light_t *l, GLuint shader, const char *prefix)
{
  if (l->is_shadow) {
    glUniform1i(ex_uniform(shader, "u_spot_light.is_shadow"), 1);
    
    glUniform1i(ex_uniform(shader, "u_spot_depth"), 5);
    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, l->depth_map);
  } else if (prefix != NULL) {
    char buff[64];
    sprintf(buff, "%s.is_shadow", prefix);
    glUniform1i(ex_uniform(shader, buff), 0);
  }

  if (prefix != NULL) {
    char buff[64];
    sprintf(buff, "%s.far", prefix);
    glUniform1f(ex_uniform(shader,  buff), EX_SPOT_FAR_PLANE);
    sprintf(buff, "%s.position", prefix);
    glUniform3fv(ex_uniform(shader, buff), 1, l->position);
    sprintf(buff, "%s.color", prefix);
    glUniform3fv(ex_uniform(shader, buff), 1, l->color);
  } else {
    glUniform1i(ex_uniform(shader,  "u_spot_active"), 1);
    glUniform1f(ex_uniform(shader,  "u_spot_light.far"), EX_SPOT_FAR_PLANE);
    glUniform3fv(ex_uniform(shader, "u_spot_light.position"), 1, l->position);
    glUniform3fv(ex_uniform(shader, "u_spot_light.direction"), 1, l->direction);
    glUniform3fv(ex_uniform(shader, "u_spot_light.color"), 1, l->color);
    glUniform1f(ex_uniform(shader,  "u_spot_light.inner"), l->inner);
    glUniform1f(ex_uniform(shader,  "u_spot_light.outer"), l->outer);
  }
}

void ex_spot_light_destroy(ex_spot_light_t *l)
{
  glDeleteFramebuffers(1, &l->depth_map_fbo);
  glDeleteTextures(1, &l->depth_map);
  free(l);
}