#include <camera.h>
#include <window.h>
#include <stdlib.h>

iso_camera_t *iso_camera_new(float x, float y, float z, float sensitivity, float fov)
{
  iso_camera_t* cam = malloc(sizeof(iso_camera_t));

  cam->position[0] = 0.0f+x;
  cam->position[1] = 0.0f+y;
  cam->position[2] = 0.0f+z;
  
  cam->front[0] = 0.0f;
  cam->front[1] = 0.0f;
  cam->front[2] = -1.0f;
  
  cam->up[0] = 0.0f;
  cam->up[1] = 1.0f;
  cam->up[2] = 0.0f;

  cam->yaw    = 0.0f;
  cam->pitch  = 0.0f;
  cam->fov    = fov;
  cam->sensitivity = sensitivity;

  mat4x4_identity(cam->view);
  mat4x4_identity(cam->projection);

  // setup projection
  iso_camera_resize(cam);

  return cam;
}

void iso_camera_resize(iso_camera_t *cam)
{
  int width, height;
  glfwGetFramebufferSize(display.window, &width, &height);
  mat4x4_perspective(cam->projection, rad(cam->fov), (float)width / (float)height, 0.1f, 100.0f);
}

void iso_camera_update(iso_camera_t *cam, GLuint shader_program)
{
  /* update front vector */
  vec3 front;
  front[0] = cos(rad(cam->yaw)) * cos(rad(cam->pitch));
  front[1] = sin(rad(cam->pitch));
  front[2] = sin(rad(cam->yaw)) * cos(rad(cam->pitch));
  
  vec3_norm(camera->front, front);

  vec3 center;
  vec3_add(center, cam->position, cam->front);
  mat4x4_look_at(cam->view, cam->position, center, cam->up);

  // send vars to shader
  GLuint projection_location = glGetUniformLocation(shader_program, "u_projection");
  glUniformMatrix4fv(projection_location, 1, GL_FALSE, cam->projection[0]);
  GLuint view_location = glGetUniformLocation(shader_program, "u_view");
  glUniformMatrix4fv(view_location, 1, GL_FALSE, cam->view[0]);
  GLuint viewp_location = glGetUniformLocation(shader_program, "u_view_position");
  glUniform3f(viewp_location, 1, GL_FALSE, cam->position[0]);
}