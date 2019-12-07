#include <stdlib.h>
#include <string.h>
#include "render/dirlight.h"
#include "render/shader.h"

#define SHADOW_MAP_SIZE 1024
#define DIR_FAR_PLANE   50
#define DIR_LIGHT_SIZE  15
mat4x4 dir_shadow_projection;
GLuint ex_dir_light_shader;

void ex_dir_light_init()
{
  float s = DIR_LIGHT_SIZE;
  ex_dir_light_shader = ex_shader("dirfbo.glsl");
  mat4x4_ortho(dir_shadow_projection, -s, s, -s, s, 0.1f, DIR_FAR_PLANE);
}

ex_dir_light_t* ex_dir_light_new(vec3 pos, vec3 color, int dynamic)
{
  ex_dir_light_t *l = malloc(sizeof(ex_dir_light_t));

  memcpy(l->position, pos, sizeof(vec3));
  memcpy(l->color, color, sizeof(vec3));
  memset(l->cposition, 0, sizeof(vec3));

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
    printf("Error! Dir light framebuffer is not complete!\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  l->shader  = ex_dir_light_shader;
  l->dynamic = dynamic;
  l->update  = 1;

  return l;
}

void ex_dir_light_begin(ex_dir_light_t *l)
{
  l->update = 0;

  // set projection
  vec3 pos, target;
  vec3_add(pos, l->position, l->cposition);
  vec3_sub(target, l->cposition, l->position);
  mat4x4_look_at(l->transform, pos, target, (vec3){0.0f, 1.0f, 0.0f});
  mat4x4_mul(l->transform, dir_shadow_projection, l->transform);
  memcpy(l->target, target, sizeof(vec3));

  glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
  glBindFramebuffer(GL_FRAMEBUFFER, l->depth_map_fbo);

  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glUseProgram(l->shader);

  glUniformMatrix4fv(ex_uniform(l->shader, "u_light_transform"), 1, GL_FALSE, &l->transform[0][0]);
}

void ex_dir_light_draw(ex_dir_light_t *l, GLuint shader)
{
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, l->depth_map);
  glUniform1i(ex_uniform(shader, "u_dir_depth"), 4);

  vec3 temp;
  vec3_add(temp, l->cposition, l->position);
  glUniformMatrix4fv(ex_uniform(shader, "u_dir_transform"), 1, GL_FALSE, &l->transform[0][0]);
  glUniform3fv(ex_uniform(shader, "u_dir_light.position"), 1, temp);
  glUniform3fv(ex_uniform(shader, "u_dir_light.target"), 1, l->target);
  glUniform3fv(ex_uniform(shader, "u_dir_light.color"), 1, l->color);
  glUniform1f(ex_uniform(shader, "u_dir_light.far"), DIR_FAR_PLANE);
}

void ex_dir_light_destroy(ex_dir_light_t *l)
{
  glDeleteFramebuffers(1, &l->depth_map_fbo);
  glDeleteTextures(1, &l->depth_map);
  free(l);
}