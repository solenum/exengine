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
  list_t *mesh_list;

  vec3 position, rotation;
  float scale;
  uint8_t is_lit, is_shadow;

  anim_t   *current_anim;
  float    current_time;
  uint32_t current_frame;

  mat4x4 transform, *inverse_base, *skeleton;
  bone_t *bones;
  anim_t *anims;
  frame_t *frames, bind_pose, pose;
  size_t bones_len, anims_len, frames_len;
  int use_transform;

  vec3 *vertices;
  size_t num_vertices;

  octree_t *octree_data;
} model_t;

model_t* model_new();

void model_update(model_t *m, float delta_time);

void model_draw(model_t *m, GLuint shader);

void model_destroy(model_t *m);

/**
 * [model_update_matrices updates bone matrices]
 * @param m [model_t pointer]
 */
void model_update_matrices(model_t *m);

/**
 * [model_set_pose sets skeleton pose]
 * @param m     [model_t pointer]
 * @param frame [frame data to set as]
 */
void model_set_pose(model_t *m, frame_t frame);

/**
 * [model_set_anim sets anim for given index]
 * @param m     [model_t pointer]
 * @param index [animation index]
 */
void model_set_anim(model_t *m, size_t index);

void model_get_bone_transform(model_t *m, const char *name, mat4x4 transform);

/**
 * [calc_bone_matrix gets bone matrix based on given input]
 * @param m     [model_t pointer]
 * @param pos   [vec3 position]
 * @param rot   [quat rotation]
 * @param scale [vec3 scale]
 */
void calc_bone_matrix(mat4x4 m, vec3 pos, quat rot, vec3 scale);

/**
 * [mix_pose transposes between two frames]
 * @param m      [model_t pointer]
 * @param a      [frame a]
 * @param b      [frame b]
 * @param weight [how much to transpose (0 to 1.0)]
 */
void mix_pose(model_t *m, frame_t a, frame_t b, float weight);


#endif // EX_MODEL_H