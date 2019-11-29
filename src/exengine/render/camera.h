/* camera
  Handles creating and updating various types
  of cameras.
*/

#ifndef EX_CAMERA_H
#define EX_CAMERA_H

#include "glad/glad.h"

#include "math/mathlib.h"
#include "render/model.h"

typedef struct {
  mat4x4 view, projection, inverse_view;
} ex_camera_matrices_t;

typedef struct {
  vec3 position, front, up;
  float yaw, pitch, last_x, last_y, fov, sensitivity;
  int width, height, update;
  ex_model_t *view_model;
  ex_camera_matrices_t matrices;
} ex_fps_camera_t;

/**
 * [ex_fps_camera_new create a first person camera]
 * @param  x [x position]
 * @param  y [y position]
 * @param  z [z position]
 */
ex_fps_camera_t* ex_fps_camera_new(float x, float y, float z, float sensitivity, float fov);

/**
 * [ex_fps_camera_resize adjust the projection matrices]
 * @param cam [camera to resize]
 *
 * To be called right after scene_draw, but only if
 * the specified scene width and height where 0x0.
 */
void ex_fps_camera_resize(ex_fps_camera_t *cam);

/**
 * [ex_fps_camera_update handle input and rotation]
 * @param cam            [camera to update]
 * @param xrel           [x motion]
 * @param yrel           [y motion]
 */
void ex_fps_camera_update(ex_fps_camera_t *cam);

#endif // EX_CAMERA_H