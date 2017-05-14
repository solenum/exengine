#ifndef IQM_LOADER_H
#define IQM_LOADER_H

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "scene.h"
#include "model.h"

#define uint uint32_t
#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

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
} iqm_header_t;

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
} iqmadjacency_t;

typedef struct {
  uint name;
  int parent;
  float translate[3], rotate[4], scale[3];
} iqmjoint_t;

typedef struct {
  int parent;
  uint channelmask;
  float channeloffset[10];
  float channelscale[10];
} iqmpose_t;

typedef struct {
  uint name;
  uint first_frame, num_frames;
  float framerate;
  uint flags;
} iqmanim_t;

typedef struct {
  uint type;
  uint flags;
  uint format;
  uint size;
  uint offset;
} iqmvertexarray_t;

typedef struct {
  float bbmin[3], bbmax[3];
  float xyradius, radius;
} iqmbounds_t;

typedef struct {
  uint name;
  uint material;
  uint first_vertex, num_vertexes;
  uint first_triangle, num_triangles;
} iqmmesh_t;

model_t *iqm_load_model(scene_t *scene, const char *path);

static inline uint get_uint(uint8_t *data) { 
  return (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

#endif // IQM_LOADER_H