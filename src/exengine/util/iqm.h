/* iqm
  Loads IQM files and defines a model
  from its data.

  This supports all IQM features, including
  skeleton animation.  It also internally
  uses the cache system.
*/

#ifndef EX_IQM_LOADER_H
#define EX_IQM_LOADER_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render/scene.h"
#include "render/model.h"

#define uint uint32_t
#define EX_IQM_MAGIC "INTERQUAKEMODEL"
#define EX_IQM_VERSION 2

/*
  The loader flags, OR these against
  eachother in the flags argument to
  specify what you want the loader
  to do with the model file.

  if EX_KEEP_VERTICES is defined,
  the loader will add the model vertices
  to the scenes collision tree.
*/
#define EX_KEEP_VERTICES 1

typedef struct {
  char magic[16];
  uint version;
  uint filesize;
  uint flags;
  uint num_text, ofs_text;
  uint num_meshes, ofs_meshes;
  uint num_vertexarrays, num_vertexes, ofs_vertexarrays;
  uint num_triangles, ofs_triangles, ofs_adjacency;
  uint num_joints, ofs_joints;
  uint num_poses, ofs_poses;
  uint num_anims, ofs_anims;
  uint num_frames, num_framechannels, ofs_frames, ofs_bounds;
  uint num_comment, ofs_comment;
  uint num_extensions, ofs_extensions;
} ex_iqm_header_t;

enum {
  IQM_POSITION     = 0,
  IQM_TEXCOORD     = 1,
  IQM_NORMAL       = 2,
  IQM_TANGENT      = 3,
  IQM_BLENDINDEXES = 4,
  IQM_BLENDWEIGHTS = 5,
  IQM_COLOR        = 6,
  IQM_CUSTOM       = 0x10,
  IQM_BYTE   = 0,
  IQM_UBYTE  = 1,
  IQM_SHORT  = 2,
  IQM_USHORT = 3,
  IQM_INT    = 4,
  IQM_UINT   = 5,
  IQM_HALF   = 6,
  IQM_FLOAT  = 7,
  IQM_DOUBLE = 8,
  IQM_LOOP   = 1<<0
};

typedef struct {
  uint triangle[3];
} ex_iqmadjacency_t;

typedef struct {
  uint name;
  int parent;
  float translate[3], rotate[4], scale[3];
} ex_iqmjoint_t;

typedef struct {
  int parent;
  uint channelmask;
  float channeloffset[10];
  float channelscale[10];
} ex_iqmex_pose_t;

typedef struct {
  uint name;
  uint first_frame, num_frames;
  float framerate;
  uint flags;
} ex_iqmex_anim_t;

typedef struct {
  uint type;
  uint flags;
  uint format;
  uint size;
  uint offset;
} ex_iqmvertexarray_t;

typedef struct {
  float bbmin[3], bbmax[3];
  float xyradius, radius;
} ex_iqmbounds_t;

typedef struct {
  uint name;
  uint material;
  uint first_vertex, num_vertexes;
  uint first_triangle, num_triangles;
} ex_iqmex_mesh_t;

/**
 * [ex_iqm_load_model loads a given iqm model file]
 * @param  scene [required if keep vertices is specified in flags]
 * @param  path  [path to the model file]
 * @param  flags [see flag defines above]
 * @return       [an instance of the requested iqm model]
 */
ex_model_t *ex_iqm_load_model(ex_scene_t *scene, const char *path, uint8_t flags);

static inline uint ex_get_uint(uint8_t *data) { 
  return (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

static void ex_iqm_get_args(const char *str, vec4 args) {
  char *end;
  args[0] = strtof(str, &end);
  args[1] = strtof(&end[1], &end);
  args[2] = strtof(&end[1], &end);
  args[3] = strtof(&end[1], &end);
}

#endif // EX_IQM_LOADER_H