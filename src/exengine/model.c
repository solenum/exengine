#include <string.h>
#include "model.h"

ex_model_t* ex_model_new()
{
  // init lists etc
  ex_model_t *m = malloc(sizeof(ex_model_t));
  m->mesh_list = list_new();

  // init attributes
  memset(m->position, 0, sizeof(vec3));
  memset(m->rotation, 0, sizeof(vec3));
  m->scale     = 1.0f;
  m->is_shadow = 1;
  m->is_lit    = 1;
  m->use_transform  = 0;
  mat4x4_identity(m->transform);

  m->current_anim  = NULL;
  m->current_time  = 0.0;
  m->current_frame = 0;

  return m;
}

void ex_model_update(ex_model_t *m, float delta_time)
{
  // update mesh transform
  list_node_t *n = m->mesh_list;
  while (n->data != NULL) {
    // update attributes 
    ex_mesh_t *mesh = n->data;
    memcpy(mesh->position, m->position, sizeof(vec3));
    memcpy(mesh->rotation, m->rotation, sizeof(vec3));
    mesh->scale  = m->scale;
    mesh->is_lit = m->is_lit;

    mesh->use_transform = m->use_transform;
    if (m->use_transform)
      mat4x4_dup(mesh->transform, m->transform);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // handle animations
  ex_anim_t *anim = m->current_anim;

  if (anim == NULL)
    return;
  
  // get current frame
  uint32_t current_frame = m->current_time * anim->rate;
  uint32_t len = anim->last + anim->first;
  float position = m->current_time * anim->rate;
  
  if (current_frame > len && !anim->loop)
    return;

  // increase frame time
  m->current_time += delta_time;
  m->current_frame = anim->first + current_frame;
  int next_frame = m->current_frame+1;

  // check frame bounds
  if (m->current_frame >= len) {
    if (anim->loop) {
      m->current_time -= len / anim->rate;
      m->current_time  = 0;
      m->current_frame = anim->first + m->current_time * anim->rate;
    } else {
      m->current_frame = anim->last;
    }
  }

  if (next_frame >= len) {
    next_frame = anim->first;
  }

  // update skeleton matrices
  ex_mix_pose(m, m->frames[m->current_frame], m->frames[next_frame], position - (float)floor(position));

  ex_model_update_matrices(m);
}

void ex_model_draw(ex_model_t *m, GLuint shader)
{
  // pass bone data
  GLuint has_skeleton_loc = glGetUniformLocation(shader, "u_has_skeleton");
  glUniform1i(has_skeleton_loc, 0);

  if (m->bones != NULL && m->current_anim != NULL) {
    glUniform1i(has_skeleton_loc, 1);
    
    GLuint bone_loc = glGetUniformLocation(shader, "u_bone_matrix");
    glUniformMatrix4fv(bone_loc, m->bones_len, GL_TRUE, &m->skeleton[0][0][0]);
  }

  // render meshes
  list_node_t *n = m->mesh_list;
  while (n->data != NULL) {
    ex_mesh_draw(n->data, shader);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

void ex_model_destroy(ex_model_t *m)
{
  // cleanup meshes
  list_node_t *n = m->mesh_list;
  while (n->data != NULL) {
    ex_mesh_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free mesh list
  list_destroy(m->mesh_list);

  // clean up anim data
  if (m->bones != NULL)
    free(m->bones);

  if (m->anims != NULL)
    free(m->anims);

  if (m->bind_pose != NULL)
    free(m->bind_pose);

  if (m->pose != NULL)
    free(m->pose);

  if (m->frames != NULL) {
    for (int i=0; i<m->frames_len; i++)
      free(m->frames[i]);
    
    free(m->frames);
  }

  if (m->inverse_base != NULL)
    free(m->inverse_base);

  if (m->skeleton != NULL)
    free(m->skeleton);

  if (m->vertices != NULL)
    free(m->vertices);

  // free model
  free(m);
}

void ex_model_update_matrices(ex_model_t *m)
{
  mat4x4 transform[m->bones_len];
  ex_frame_t pose = m->pose;

  for (int i=0; i<m->bones_len; i++) {
    ex_bone_t b = m->bones[i];

    mat4x4 mat, result;
    ex_calc_bone_matrix(mat, pose[i].translate, pose[i].rotate, pose[i].scale);
    mat4x4_identity(result);

    if (b.parent >= 0) {
      mat4x4_mul(transform[i], mat, transform[b.parent]);
      mat4x4_mul(result, m->inverse_base[i], transform[i]);
    } else {
      mat4x4_dup(transform[i], mat);
      mat4x4_mul(result, m->inverse_base[i], mat);
    }

    mat4x4_dup(m->bones[i].transform, transform[i]);
    mat4x4_dup(m->skeleton[i], result);
  }
}

void ex_model_set_pose(ex_model_t *m, ex_frame_t frame)
{
  for (int i=0; i<m->bones_len; i++) {
    ex_pose_t f = frame[i];
    
    quat rotate;
    memcpy(rotate, f.rotate, sizeof(quat));
    quat_norm(rotate, rotate);

    memcpy(m->pose[i].translate,  f.translate, sizeof(vec3));
    memcpy(m->pose[i].rotate,     rotate,      sizeof(quat));
    memcpy(m->pose[i].scale,      f.scale,     sizeof(vec3));
  }
}

void ex_model_set_anim(ex_model_t *m, size_t index)
{
  if (index > m->anims_len) {
    m->current_anim = NULL;
    return;
  }

  m->current_anim  = &m->anims[index];
  m->current_time  = 0;
  m->current_frame = m->current_anim->first;
}

void ex_model_get_ex_bone_transform(ex_model_t *m, const char *name, mat4x4 transform)
{
  mat4x4 temp;
  mat4x4_identity(transform);
  mat4x4_identity(temp);

  // get bone by name
  for (int i=0; i<m->bones_len; i++) {
    if (strcmp(m->bones[i].name, name) == 0) {
      // apply model transforms
      if (!m->use_transform) {
        mat4x4_translate_in_place(temp, m->position[0], m->position[1], m->position[2]);
        // mat4x4_rotate_Y(temp, temp, rad(m->rotation[1]));
        // mat4x4_rotate_X(temp, temp, rad(m->rotation[0]));
        // mat4x4_rotate_Z(temp, temp, rad(m->rotation[2]));
        // mat4x4_scale_aniso(temp, temp, m->scale, m->scale, m->scale);
      } else {
        // mat4x4_mul(temp, m->transform, temp);
      }

      // apply bone transform
      mat4x4 mat = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
      };
      mat4x4_mul(transform, mat, m->bones[i].transform);
      // mat4x4_mul(transform, temp, transform);
      // mat4x4_mul(transform, m->bones[i].transform, m->inverse_base[i]);
      // mat4x4_mul(transform, transform, m->bones[i].transform);
      // mat4x4_dup(transform, m->bones[i].transform);
      // mat4x4_mul(transform, transform, m->inverse_base[i]);
      return;
    }
  }
}

void ex_calc_bone_matrix(mat4x4 m, vec3 pos, quat rot, vec3 scale)
{
  mat4x4 mat;

  mat4x4_identity(m);

  mat4x4_scale_xyz(mat, scale);
  mat4x4_mul(m, m, mat);

  mat4x4_rotate_quat(mat, rot);
  mat4x4_mul(m, m, mat);

  mat4x4_translate(mat, pos);
  mat4x4_mul(m, m, mat);
}

void ex_mix_pose(ex_model_t *m, ex_frame_t a, ex_frame_t b, float weight)
{
  weight = MIN(MAX(weight, 0.0f), 1.0f);
  for (int i=0; i<m->bones_len; i++) {
    vec3 t;
    vec3_lerp(t, a[i].translate, b[i].translate, weight);

    quat r;
    quat_slerp(r, a[i].rotate, b[i].rotate, weight);
    quat_norm(r, r);

    vec3 s;
    vec3_lerp(s, a[i].scale, b[i].scale, weight);

    memcpy(m->pose[i].translate,  t, sizeof(vec3));
    memcpy(m->pose[i].rotate,     r, sizeof(quat));
    memcpy(m->pose[i].scale,      s, sizeof(vec3));
  }
}