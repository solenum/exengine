#ifndef BILLBOARD_H
#define BILLBOARD_H

#include <mesh.h>
#include <mathlib.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct {
  mesh_t *mesh;
  vec3 position;
  GLuint shader_program;
  float width, height, scale;
  double frame_timer, frame_speed;
  int frame_start, frame_end, frame;
  bool animate;
  texture_t texture;
} billboard_t;

/**
 * [billboard_new generates a new billboard sprite thingy]
 * @param  shader_program [shader program id]
 * @param  texture        [texture_t var]
 * @param  width          [sprite width]
 * @param  height         [sprite height]
 * @param  scale          [world scale]
 * @return                [billboard_t pointer]
 */
billboard_t *billboard_new(GLuint shader_program, texture_t texture, float width, float height, float scale);

/**
 * [billboard_animate animates a billboard]
 * @param b [billboard_t pointer]
 */
void billboard_animate(billboard_t *b);

/**
 * [billboard_draw renders the billboard]
 * @param b [billboard_t pointer]
 */
void billboard_draw(billboard_t *b);

/**
 * [billboard_set_position updates the billboards world position]
 * @param b [billboard_t pointer]
 * @param x [x position]
 * @param y [y position]
 * @param z [z position]
 */
void billboard_set_position(billboard_t *b, float x, float y, float z);

/**
 * [billboard_destroy cleans up data]
 * @param b [billboard_t pointer]
 */
void billboard_destroy(billboard_t *b);

#endif // BILLBOARD_H