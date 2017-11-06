#ifndef EX_MODEL_H
#define EX_MODEL_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "exe_list.h"
#include "octree.h"
#include "mesh.h"

typedef struct {
  char name[64];
  int parent;
  vec3 position, scale;
  quat rotation;
  mat4x4 transform;
} ex_bone_t;

typedef struct {
  uint32_t name, first, last;
  float rate;
  uint8_t loop;
} ex_anim_t;

typedef struct {
  vec3 translate, scale;
  quat rotate;
} ex_pose_t;

typedef ex_pose_t* ex_frame_t;

typedef struct {
  list_t *mesh_list;

  vec3 position, rotation;
  float scale;
  uint8_t is_lit, is_shadow;

  ex_anim_t   *current_anim;
  float    current_time;
  uint32_t current_frame;

  mat4x4 transform, *inverse_base, *skeleton;
  ex_bone_t *bones;
  ex_anim_t *anims;
  ex_frame_t *frames, bind_pose, pose;
  size_t bones_len, anims_len, frames_len;
  int use_transform;

  vec3 *vertices;
  size_t num_vertices;

  ex_octree_t *octree_data;
} ex_model_t;

ex_model_t* ex_model_new();

void ex_model_update(ex_model_t *m, float delta_time);

void ex_model_draw(ex_model_t *m, GLuint shader);

void ex_model_destroy(ex_model_t *m);

/**
 * [ex_model_update_matrices updates bone matrices]
 * @param m [ex_model_t pointer]
 */
void ex_model_update_matrices(ex_model_t *m);

/**
 * [ex_model_set_pose sets skeleton pose]
 * @param m     [ex_model_t pointer]
 * @param frame [frame data to set as]
 */
void ex_model_set_pose(ex_model_t *m, ex_frame_t frame);

/**
 * [ex_model_set_anim sets anim for given index]
 * @param m     [ex_model_t pointer]
 * @param index [animation index]
 */
void ex_model_set_anim(ex_model_t *m, size_t index);

void ex_model_get_ex_bone_transform(ex_model_t *m, const char *name, mat4x4 transform);

/**
 * [ex_calc_bone_matrix gets bone matrix based on given input]
 * @param m     [ex_model_t pointer]
 * @param pos   [vec3 position]
 * @param rot   [quat rotation]
 * @param scale [vec3 scale]
 */
void ex_calc_bone_matrix(mat4x4 m, vec3 pos, quat rot, vec3 scale);

/**
 * [ex_mix_pose transposes between two frames]
 * @param m      [ex_model_t pointer]
 * @param a      [frame a]
 * @param b      [frame b]
 * @param weight [how much to transpose (0 to 1.0)]
 */
void ex_mix_pose(ex_model_t *m, ex_frame_t a, ex_frame_t b, float weight);


#endif // EX_MODEL_H