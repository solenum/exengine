#include "camera.h"
#include "window.h"
#include <stdlib.h>
#include <string.h>

fps_camera_t* fps_camera_new(float x, float y, float z, float sensitivity, float fov)
{
  fps_camera_t *c = malloc(sizeof(fps_camera_t));

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

void fps_camera_resize(fps_camera_t *cam)
{
  int width, height;
  glfwGetFramebufferSize(display.window, &width, &height);
  
  if (cam->width != width || cam->height != height) {
    mat4x4_perspective(cam->projection, rad(cam->fov), (float)width / (float)height, 0.01f, 1000.0f);
    cam->width  = width;
    cam->height = height;
  }
}

void fps_camera_update(fps_camera_t *cam, GLuint shader_program)
{
  fps_camera_resize(cam);

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

  // update view model
  if (cam->view_model != NULL) {
    cam->view_model->is_viewmodel = 1;
    mat4x4_identity(cam->view_model_transform);

    // this is borderline retarded
    mat4x4_translate_in_place(cam->view_model_transform, cam->position[0], cam->position[1], cam->position[2]);
    mat4x4_translate_in_place(cam->view_model_transform, cam->view_model_offset[0], cam->view_model_offset[1], cam->view_model_offset[2]);
    mat4x4_rotate_Y(cam->view_model_transform, cam->view_model_transform, rad(cam->yaw + 90.0f));
    mat4x4_rotate_X(cam->view_model_transform, cam->view_model_transform, rad(cam->pitch));

    memcpy(cam->view_model->position, cam->view_model_offset, sizeof(vec3));
  }
}

void fps_camera_draw(fps_camera_t *cam, GLuint shader_program)
{
  // send vars to shader
  GLuint projection_location = glGetUniformLocation(shader_program, "u_projection");
  glUniformMatrix4fv(projection_location, 1, GL_FALSE, cam->projection[0]);
  GLuint view_location = glGetUniformLocation(shader_program, "u_view");
  glUniformMatrix4fv(view_location, 1, GL_FALSE, cam->view[0]);
  GLuint viewp_location = glGetUniformLocation(shader_program, "u_view_position");
  glUniform3fv(viewp_location, 1, &cam->position[0]);
  GLuint viewm_location = glGetUniformLocation(shader_program, "u_view_model_transform");
  glUniformMatrix4fv(viewm_location, 1, GL_FALSE, cam->view_model_transform[0]);
}