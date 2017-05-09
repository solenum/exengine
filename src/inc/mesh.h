#ifndef MESH_H
#define MESH_H

#include <mathlib.h>

#define GLEW_STATIC
#include <GL/glew.h>

typedef struct {
  float position[3];
  float uv[2];
  float normal[3];
  float tangent[4];
  uint8_t color[4];
  uint8_t blend_indexes[4];
  uint8_t blend_weights[4];
} vertex_t;

typedef struct {
  uint32_t name;
  int parent;
  vec3 position, scale;
  quat rotation;
} bone_t;

typedef struct {
  uint32_t name, first, last;
  float rate;
  uint8_t loop;
} anim_t;

typedef struct {
  vec3 translate, scale;
  quat rotate;
} pose_t;

typedef pose_t* frame_t;

typedef struct {
  GLuint texture, VAO, VBO, EBO, vcount, icount;
  mat4x4 transform, *inverse_base, *skeleton;
  bone_t *bones;
  anim_t *anims;
  frame_t *frames, bind_pose, pose;
  size_t bones_len, anims_len, frames_len;

  anim_t   *current_anim;
  float    current_time;
  uint32_t current_frame;

  vec3 position, rotation;
  float scale;
  uint8_t is_lit;
} mesh_t;

/**
 * [mesh_new generate a mesh with the given vertices, indices, and texture]
 * @param  vertices [pointer to vertices]
 * @param  vcount   [length of vertices]
 * @param  indices  [pointer to indces]
 * @param  icount   [length of indices]
 * @param  texture  [the texture's uint]
 * @return          [mesh_t pointer]
 */
mesh_t* mesh_new(vertex_t *vertices, size_t vcount, GLuint *indices, size_t icount, GLuint texture);

/**
 * [mesh_draw renders a mesh to the screen]
 * @param m              [mesh_t pointer]
 * @param shader_program [shader program to use]
 */
void mesh_draw(mesh_t* m, GLuint shader_program);

/**
 * [mesh_destroy free any malloc'd data]
 * @param m [mesh_t pointer]
 */
void mesh_destroy(mesh_t *m);

/**
 * [mesh_update updates animation etc]
 * @param m          [mesh_t pointer]
 * @param delta_time [frame time]
 */
void mesh_update(mesh_t *m, float delta_time);

/**
 * [mesh_update_matrices updates bone matrices]
 * @param m [mesh_t pointer]
 */
void mesh_update_matrices(mesh_t *m);

/**
 * [mesh_set_pose sets skeleton pose]
 * @param m     [mesh_t pointer]
 * @param frame [frame data to set as]
 */
void mesh_set_pose(mesh_t *m, frame_t frame);

/**
 * [mesh_set_anim sets anim for given index]
 * @param m     [mesh_t pointer]
 * @param index [animation index]
 */
void mesh_set_anim(mesh_t *m, size_t index);

/**
 * [calc_bone_matrix gets bone matrix based on given input]
 * @param m     [mesh_t pointer]
 * @param pos   [vec3 position]
 * @param rot   [quat rotation]
 * @param scale [vec3 scale]
 */
void calc_bone_matrix(mat4x4 m, vec3 pos, quat rot, vec3 scale);

/**
 * [mix_pose transposes between two frames]
 * @param m      [mesh_t pointer]
 * @param a      [frame a]
 * @param b      [frame b]
 * @param weight [how much to transpose (0 to 1.0)]
 */
void mix_pose(mesh_t *m, frame_t a, frame_t b, float weight);

#endif // MESH_H