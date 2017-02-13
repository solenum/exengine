#include <iqm.h>
#include <io.h>
#include <string.h>

mesh_t *iqm_load_model(const char *path)
{
  printf("Loading IQM model file %s\n", path);

  // read in the file data
  uint8_t *data = io_read_file(path, "rb");
  if (data == NULL) {
    printf("Failed to load IQM model file %s\n", path);
    return NULL;
  }

  // the header contents
  iqm_header_t header;

  // check magic string and version
  memcpy(header.magic, data, 16);
  uint *head = (uint *)&data[16];
  header.version = head[0];
  if (strcmp(header.magic, IQM_MAGIC) != 0 || header.version != IQM_VERSION) {
    printf("Loaded IQM model version is not 2.0\nFailed loading %s\n", path);
    free(data);
    return NULL;
  }

  // get the rest of the header weeeeee
  header.filesize           = head[1];
  header.flags              = head[2];
  header.num_text           = head[3];
  header.ofs_text           = head[4];
  header.num_meshes         = head[5];
  header.ofs_meshes         = head[6];
  header.num_vertexarrays   = head[7];
  header.num_vertexes       = head[8];
  header.ofs_vertexarrays   = head[9];
  header.num_triangles      = head[10];
  header.ofs_triangles      = head[11];
  header.ofs_adjacency      = head[12];
  header.num_joints         = head[13];
  header.ofs_joints         = head[14];
  header.num_poses          = head[15];
  header.ofs_poses          = head[16];
  header.num_anims          = head[17];
  header.ofs_anims          = head[18];
  header.num_frames         = head[19];
  header.num_framechannels  = head[20];
  header.ofs_frames         = head[21];
  header.ofs_bounds         = head[22];
  header.num_comment        = head[23];
  header.ofs_comment        = head[24];
  header.num_extensions     = head[25];
  header.ofs_extensions     = head[26];

  iqmmesh_t *meshes = (iqmmesh_t *)&data[header.ofs_meshes];

  // set the vertices
  vertex_t *vertices  = malloc(sizeof(vertex_t)*(header.num_vertexes));
  float *position, *uv, *normal, *tangent;
  uint8_t *blend_indexes, *blend_weights, *color;
  iqmvertexarray_t *vas = (iqmvertexarray_t *)&data[header.ofs_vertexarrays];
  for (int i=0; i<header.num_vertexarrays; i++) {
    iqmvertexarray_t va = vas[i];

    switch (va.type) {
      case IQM_POSITION: {
        position = (float *)&data[va.offset];
        for (int x=0; x<header.num_vertexes; x++)
          memcpy(&vertices[x].position, &position[x*va.size], va.size*sizeof(float));
        break;
      }
      case IQM_TEXCOORD: {
        uv = (float *)&data[va.offset];
        for (int x=0; x<header.num_vertexes; x++)
          memcpy(&vertices[x].uv, &uv[x*va.size], va.size*sizeof(float));
        break;
      }
      case IQM_NORMAL: {
        normal = (float *)&data[va.offset];
        for (int x=0; x<header.num_vertexes; x++)
          memcpy(&vertices[x].normal, &normal[x*va.size], va.size*sizeof(float));
        break;
      }
      case IQM_TANGENT: {
        tangent = (float *)&data[va.offset];
        for (int x=0; x<header.num_vertexes; x++)
          memcpy(&vertices[x].tangent, &tangent[x*va.size], va.size*sizeof(float));
        break;
      }
      case IQM_BLENDINDEXES: {
        blend_indexes = (uint8_t *)&data[va.offset];
        for (int x=0; x<header.num_vertexes; x++)
          memcpy(&vertices[x].blend_indexes, &blend_indexes[x*va.size], va.size*sizeof(uint8_t));
        break;
      }
      case IQM_BLENDWEIGHTS: {
        blend_weights = (uint8_t *)&data[va.offset];
        for (int x=0; x<header.num_vertexes; x++)
          memcpy(&vertices[x].blend_weights, &blend_weights[x*va.size], va.size*sizeof(uint8_t));
        break;
      }
      case IQM_COLOR: {
        color = (uint8_t *)&data[va.offset];
        for (int x=0; x<header.num_vertexes; x++)
          memcpy(&vertices[x].color, &color[x*va.size], va.size*sizeof(uint8_t));
        break;
      }
    }
  }

  // bones and joints
  bone_t *bones       = NULL;
  frame_t *bind_pose  = NULL;
  frame_t *pose       = NULL;
  iqmjoint_t *joints  = (iqmjoint_t *)&data[header.ofs_joints];
  if (header.ofs_joints > 0) {
    bones     = malloc(sizeof(bone_t)*header.num_joints);
    bind_pose = malloc(sizeof(bone_t)*header.num_joints);
    pose      = malloc(sizeof(bone_t)*header.num_joints);
    for (int i=0; i<header.num_joints; i++) {
      iqmjoint_t *j   = &joints[i];
      bones[i].name   = j->name;
      bones[i].parent = j->parent;
      memcpy(bones[i].position, j->translate, sizeof(vec3));
      memcpy(bones[i].rotation, j->rotate,    sizeof(quat));
      memcpy(bones[i].scale,    j->scale,     sizeof(vec3));
      memcpy(bind_pose[i].translate,  j->translate, sizeof(vec3));
      memcpy(bind_pose[i].rotate,     j->rotate,    sizeof(quat));
      memcpy(bind_pose[i].scale,      j->scale,     sizeof(vec3));
    }
  }

  // anims
  anim_t *anims = NULL;
  iqmanim_t *animdata = (iqmanim_t *)&data[header.ofs_anims];
  if (header.ofs_anims > 0) {
    anims = malloc(sizeof(anim_t)*header.num_anims);
    for (int i=0; i<header.num_anims; i++) {
      iqmanim_t *a    = &animdata[i];
      anims[i].name   = a->name;
      anims[i].first  = a->first_frame;
      anims[i].last   = a->first_frame+a->num_frames;
      anims[i].rate   = a->framerate;
      anims[i].loop   = a->flags || (1<<0);
    }  
  }

  // poses
  unsigned short *framedata = NULL;
  frame_t **frames = NULL;
  iqmpose_t *posedata = (iqmpose_t *)&data[header.ofs_poses];
  if (header.ofs_poses > 0) {
    frames = malloc(sizeof(frame_t)*header.num_frames);
    framedata = (unsigned short *)&data[header.ofs_frames];
    
    for (int i=0; i<header.num_frames; i++) {

      frame_t frame[header.num_poses];
      for (int p=0; p<header.num_poses; p++) {
        iqmpose_t *pose = &posedata[p];
        
        float v[10];
        for (int o=0; o<10; o++) {
          float val = pose->channeloffset[o];
          uint mask = (1 << o);
          if ((pose->channelmask & mask) > 0) {
            val += framedata[0] * pose->channelscale[o];
            framedata++;
          }
          v[o] = val;
        }

        memcpy(frame[p].translate, &v[0], sizeof(vec3));
        memcpy(frame[p].rotate,    &v[3], sizeof(quat));
        memcpy(frame[p].scale,     &v[7], sizeof(vec3));
        /*printf("F: %f %f %f \nV: %f %f %f\n", frame[p].translate[0], frame[p].translate[1], frame[p].translate[2], v[0], v[1], v[2]);
        printf("R: %f %f %f %f \nV: %f %f %f %f\n", frame[p].rotate[0], frame[p].rotate[1], frame[p].rotate[2], frame[p].rotate[3], v[3], v[4], v[5], v[6]);
        printf("S: %f %f %f \nV: %f %f %f\n", frame[p].scale[0], frame[p].scale[1], frame[p].scale[2], v[7], v[8], v[9]);*/
      }

      frames[i] = malloc(header.num_poses*sizeof(frame_t));
      memcpy(frames[i], frame, header.num_poses*sizeof(frame_t));
    }
  }

  // indices
  GLuint *indices = malloc(sizeof(GLuint)*(header.num_triangles*3));
  memcpy(&indices[0], &data[header.ofs_triangles], (header.num_triangles*3)*sizeof(GLuint));
  int i, a;
  for (i=0; i<header.num_triangles*3; i+=3) {
    a = indices[i+0];
    indices[i+0] = indices[i+2];
    indices[i+2] = a;
  }

  // create the mesh
  mesh_t *m = mesh_new(vertices, header.num_vertexes, indices, header.num_triangles*3, 2);
  m->bones      = bones;
  m->anims      = anims;
  m->frames     = frames;
  m->bones_len  = header.num_joints;
  m->anims_len  = header.num_anims;
  m->frames_len = header.num_frames;
  m->bind_pose  = bind_pose;
  m->pose       = pose;

  // calc inverse base pose
  m->inverse_base = NULL;
  m->skeleton = NULL;
  for (int i=0; i<header.num_joints; i++) {
    if (header.ofs_joints < 1)
      break;

    if (!i) {
      m->inverse_base = malloc(sizeof(mat4x4)*header.num_joints);
      m->skeleton = malloc(sizeof(mat4x4)*header.num_joints);
    }

    bone_t b = m->bones[i];

    mat4x4 mat, inv;
    calc_bone_matrix(mat, b.position, b.rotation, b.scale);
    mat4x4_invert(inv, mat);

    if (b.parent > 0) {
      mat4x4_mul(m->inverse_base[i], m->inverse_base[b.parent], inv);
    } else {
      mat4x4_dup(m->inverse_base[i], inv);
    }
  }

  if (m->bind_pose != NULL) {
    //mesh_set_pose(m, &m->frames[0][200]);
    mesh_update_matrices(m, &m->frames[0][200]);
  }

  //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  printf("Finished loading IQM model %s\n", path);

  free(vertices);
  free(indices);
  free(data);
  return m;
}