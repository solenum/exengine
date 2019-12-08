/* model
  The model container consists of
  multiple meshes, one per texture set.

  It handles skeletal animation, as
  well as the transforms for all of
  its meshes.
*/

#ifndef EX_MODEL_H
#define EX_MODEL_H

#include "glad/glad.h"

#include "util/list.h"
#include "math/octree.h"
#include "render/mesh.h"

#define EX_MODEL_MAX_MESHES 128

typedef struct {
  char name[64];
  int parent;
  vec3 position, scale;
  quat rotation;
  mat4x4 transform;
} ex_bone_t;

typedef struct {
  char *name;
  uint32_t first, last;
  float rate;
  uint8_t loop;
} ex_anim_t;

typedef struct {
  vec3 translate, scale;
  quat rotate;
} ex_pose_t;

typedef ex_pose_t* ex_frame_t;

typedef struct {
  ex_mesh_t *meshes[EX_MODEL_MAX_MESHES];

  vec3 position, rotation;
  float scale;
  uint8_t is_lit, is_shadow;

  ex_anim_t *current_anim;
  float     current_time;
  uint32_t  current_frame;

  mat4x4 *inverse_base, *skeleton;
  ex_bone_t *bones;
  ex_anim_t *anims;
  ex_frame_t *frames, bind_pose, pose;
  size_t bones_len, anims_len, frames_len;
  int use_transform;

  vec3 *vertices;
  size_t num_vertices;

  ex_octree_t *octree_data;

  mat4x4 *transforms;
  GLuint instance_vbo;
  size_t instance_count;
  int    is_static;

  char path[512];
} ex_model_t;

/**
 * [ex_model_new define a new model]
 * @return [a new, empty model]
 */
ex_model_t* ex_model_new();

/**
 * [ex_model_copy create a copy that uses on the same data]
 * @param  model [the model to copy]
 * @return       [the new instance of the given model]
 */
ex_model_t* ex_model_copy(ex_model_t *model);

/**
 * [ex_model_add_mesh add a mesh to the render list]
 * @param m    [the model]
 * @param mesh [mesh to add]
 */
void ex_model_add_mesh(ex_model_t *m, ex_mesh_t *mesh);

/**
 * [ex_model_init_instancing init the intancing transform arrays]
 * @param m     [the model to instance]
 * @param count [how many instances you want]
 */
void ex_model_init_instancing(ex_model_t *m, int count);

/**
 * [ex_model_update update the model animations, transforms etc]
 * @param m          [the model to update]
 * @param delta_time []
 */
void ex_model_update(ex_model_t *m, float delta_time);

/**
 * [ex_model_destroy cleanup model data]
 * @param m [the model to destroy]
 */
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
 * @param id [animation id]
 */
void ex_model_set_anim(ex_model_t *m, char *id);

/**
 * [ex_model_get_ex_bone_transform get a bones transform for bone attachments]
 * @param m         [the model]
 * @param name      [the bone name]
 * @param transform [the transform to return]
 *
 * This is currently broken, don't use this.
 */
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