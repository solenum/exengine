#include "camera.h"
#include "window.h"
#include "shader.h"
#include <stdlib.h>
#include <string.h>

ex_fps_camera_t* ex_fps_camera_new(float x, float y, float z, float sensitivity, float fov)
{
  ex_fps_camera_t *c = malloc(sizeof(ex_fps_camera_t));

  c->position[0] = 0.0f+x;
  c->position[1] = 0.0f+y;
  c->position[2] = 0.0f+z;
  
  c->front[0] = 0.0f;
  c->front[1] = 0.0f;
  c->front[2] = -1.0f;
  
  c->up[0] = 0.0f;
  c->up[1] = 1.0f;
  c->up[2] = 0.0f;

  c->yaw    = 0.0f;
  c->pitch  = 0.0f;
  c->fov    = fov;
  c->sensitivity = sensitivity;

  c->width  = 0;
  c->height = 0;
  c->last_x = 0;
  c->last_y = 0;

  mat4x4_identity(c->view);
  mat4x4_identity(c->projection);

  c->view_model = NULL;
  memset(c->view_model_offset, 0, sizeof(vec3));
  memset(c->view_model_rotate, 0, sizeof(vec3));

  return c;
}

void ex_fps_camera_resize(ex_fps_camera_t *cam)
{
  int width, height;
  glfwGetFramebufferSize(display.window, &width, &height);
  
  if (cam->width != width || cam->height != height) {
    mat4x4_perspective(cam->projection, rad(cam->fov), (float)width / (float)height, 0.01f, 1000.0f);
    cam->width  = width;
    cam->height = height;
  }
}

void ex_fps_camera_update(ex_fps_camera_t *cam, GLuint shader_program)
{
  ex_fps_camera_resize(cam);

  float x = display.mouse_x;
  float y = display.mouse_y;

  float offset_x = x - cam->last_x;
  float offset_y = cam->last_y - y;
  cam->last_x = x;
  cam->last_y = y;

  offset_x *= cam->sensitivity;
  offset_y *= cam->sensitivity;

  cam->yaw += offset_x;
  cam->pitch += offset_y;

  if(cam->pitch > 89.0f)
      cam->pitch = 89.0f;
  if(cam->pitch < -89.0f)
      cam->pitch = -89.0f;

  /* update front vector */
  vec3 front;
  front[0] = cos(rad(cam->yaw)) * cos(rad(cam->pitch));
  front[1] = sin(rad(cam->pitch));
  front[2] = sin(rad(cam->yaw)) * cos(rad(cam->pitch));
  
  vec3_norm(cam->front, front);

  vec3 center;
  vec3_add(center, cam->position, cam->front);
  mat4x4_look_at(cam->view, cam->position, center, cam->up);
  mat4x4_invert(cam->inverse_view, cam->view);

  // update view model
  if (cam->view_model != NULL) {
    cam->view_model->use_transform= 1;

    mat4x4 mat;
    mat4x4_identity(cam->view_model->transform);
    mat4x4_invert(mat, cam->view);
    mat4x4_mul(cam->view_model->transform, mat, cam->view_model->transform);
    mat4x4_translate_in_place(cam->view_model->transform, cam->view_model_offset[0], cam->view_model_offset[1], cam->view_model_offset[2]);
  }
}

void ex_fps_camera_draw(ex_fps_camera_t *cam, GLuint shader_program)
{
  // send vars to shader
  glUniformMatrix4fv(ex_uniform(shader_program, "u_projection"), 1, GL_FALSE, cam->projection[0]);
  glUniformMatrix4fv(ex_uniform(shader_program, "u_view"), 1, GL_FALSE, cam->view[0]);
  glUniform3fv(ex_uniform(shader_program, "u_view_position"), 1, &cam->position[0]);
  glUniformMatrix4fv(ex_uniform(shader_program, "u_inverse_view"), 1, GL_FALSE, cam->inverse_view[0]);
  glUniform3fv(ex_uniform(shader_program, "u_eye_dir"), 1, &cam->front[0]);
}