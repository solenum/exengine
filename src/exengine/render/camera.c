#include <stdlib.h>
#include <string.h>
#include "render/camera.h"
#include "render/shader.h"
#include "input/input.h"

ex_fps_camera_t* ex_fps_camera_new(float x, float y, float z, float sensitivity, float fov)
{
  ex_fps_camera_t *c = malloc(sizeof(ex_fps_camera_t));

  c->position[0] = x;
  c->position[1] = y;
  c->position[2] = z;
  
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

  c->update = 1;

  mat4x4_identity(c->matrices.view);
  mat4x4_identity(c->matrices.projection);
  mat4x4_identity(c->matrices.inverse_view);

  ex_fps_camera_resize(c);

  return c;
}

void ex_fps_camera_resize(ex_fps_camera_t *cam)
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  int width, height;
  width = viewport[2];
  height = viewport[3];
  
  if (cam->width != width || cam->height != height) {
    mat4x4_perspective(cam->matrices.projection, rad(cam->fov), (float)width / (float)height, 0.01f, 1000.0f);
    cam->width  = width;
    cam->height = height;
  }
}

void ex_fps_camera_update(ex_fps_camera_t *cam)
{
  if (!cam->update)
    return;

  float x = (float)ex_mouse_x;
  float y = (float)ex_mouse_y;

  float offset_x = x;
  float offset_y = -y;

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
  mat4x4_look_at(cam->matrices.view, cam->position, center, cam->up);
  mat4x4_invert(cam->matrices.inverse_view, cam->matrices.view);
}