#include "iqm.h"
#include "exe_io.h"
#include "pointlight.h"
#include <string.h>

model_t *iqm_load_model(scene_t *scene, const char *path, int keep_vertices)
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
  memcpy(&header, data, sizeof(iqm_header_t));

  iqmmesh_t *meshes = (iqmmesh_t *)&data[header.ofs_meshes];
  char *file_text = header.ofs_text ? (char *)&data[header.ofs_text] : "";

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
  bone_t *bones      = NULL;
  frame_t bind_pose  = NULL;
  frame_t pose       = NULL;
  iqmjoint_t *joints  = (iqmjoint_t *)&data[header.ofs_joints];
  if (header.ofs_joints > 0) {
    bones     = malloc(sizeof(bone_t)*header.num_joints);
    bind_pose = malloc(sizeof(pose_t)*header.num_joints);
    pose      = malloc(sizeof(pose_t)*header.num_joints);
    printf("Bones: ");
    for (int i=0; i<header.num_joints; i++) {
      iqmjoint_t *j   = &joints[i];
      strncpy(bones[i].name, &file_text[j->name], 64);
      printf("%s ", bones[i].name);
      bones[i].parent = j->parent;
      memcpy(bones[i].position, j->translate, sizeof(vec3));
      memcpy(bones[i].rotation, j->rotate,    sizeof(quat));
      memcpy(bones[i].scale,    j->scale,     sizeof(vec3));
      memcpy(bind_pose[i].translate,  j->translate, sizeof(vec3));
      memcpy(bind_pose[i].rotate,     j->rotate,    sizeof(quat));
      memcpy(bind_pose[i].scale,      j->scale,     sizeof(vec3));
    }
    printf("\n");
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
      anims[i].last   = a->num_frames;
      anims[i].rate   = a->framerate;
      anims[i].loop   = a->flags || (1<<0);
    }  
  }

  // poses
  unsigned short *framedata = NULL;
  frame_t *frames = NULL;
  iqmpose_t *posedata = (iqmpose_t *)&data[header.ofs_poses];
  if (header.ofs_poses > 0) {
    frames = malloc(sizeof(frame_t)*header.num_frames);
    framedata = (unsigned short *)&data[header.ofs_frames];
    
    for (int i=0; i<header.num_frames; i++) {
      pose_t *frame = malloc(header.num_poses*sizeof(pose_t));
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
      }
      frames[i] = frame;
    }
  }

  // indices
  GLuint *indices = malloc(sizeof(GLuint)*(header.num_triangles*3));
  memcpy(indices, &data[header.ofs_triangles], (header.num_triangles*3)*sizeof(GLuint));
  int i, a;
  for (i=0; i<header.num_triangles*3; i+=3) {
    a = indices[i+0];
    indices[i+0] = indices[i+2];
    indices[i+2] = a;
  }

  // create the model
  model_t *model = model_new();
  model->bones       = bones;
  model->anims       = anims;
  model->frames      = frames;
  model->bones_len   = header.num_joints;
  model->anims_len   = header.num_anims;
  model->frames_len  = header.num_frames;
  model->bind_pose   = bind_pose;
  model->pose        = pose;
  model->vertices    = NULL;
  model->octree_data = NULL;

  // calc inverse base pose
  model->inverse_base = NULL;
  model->skeleton     = NULL;
  for (int i=0; i<header.num_joints; i++) {
    if (header.ofs_joints < 1)
      break;

    if (!i) {
      model->inverse_base = malloc(sizeof(mat4x4)*header.num_joints);
      model->skeleton = malloc(sizeof(mat4x4)*header.num_joints);
    }

    bone_t b = model->bones[i];

    mat4x4 mat, inv;
    calc_bone_matrix(mat, b.position, b.rotation, b.scale);
    mat4x4_invert(inv, mat);

    if (b.parent >= 0) {
      mat4x4_mul(model->inverse_base[i], model->inverse_base[b.parent], inv);
    } else {
      mat4x4_dup(model->inverse_base[i], inv);
    }

    if (i >= header.num_joints-1)
      model_update_matrices(model);
  }

  // backup vertices of visible meshes
  vec3 *vis_vertices = malloc(sizeof(vec3)*header.num_triangles*3);
  size_t vis_len = 0;
  
  // add the meshes to the model
  GLuint index_offset = 0;
  for (int i=0; i<header.num_meshes; i++) {
    vertex_t *vert = &vertices[meshes[i].first_vertex];
    GLuint *ind    = &indices[meshes[i].first_triangle*3];

    // negative offset indices
    GLuint offset = 0;
    for (int k=0; k<meshes[i].num_triangles*3; k++) {
      ind[k] -= index_offset;

      if (ind[k] > offset)
        offset = ind[k];
    }
    index_offset += ++offset;

    // get material and texture names
    char *tex_name = &file_text[meshes[i].material];
    char *is_file = strpbrk(tex_name, ".");
    
    // handle entity spawns
    if (tex_name[0] == 'e' && tex_name[1] == '.') {
      char *arg_start = strpbrk(&tex_name[2], "!");

      if (arg_start == NULL)
        continue;

      size_t name_len = strlen(&tex_name[2]) - strlen(arg_start);
      vec4 args;

      if (strncmp(&tex_name[2], "pointlight", name_len) == 0) {
        iqm_get_args(&arg_start[1], args);
        point_light_t *l = point_light_new(vert[0].position, (vec3){args[0], args[1], args[2]}, (int)args[3]);
        // list_add(scene->point_light_list, l);
      }
    } else {
      // create mesh
      mesh_t *m = mesh_new(vert, meshes[i].num_vertexes, ind, meshes[i].num_triangles*3, 0);

      // store vertices
      if (keep_vertices) {
        size_t size = meshes[i].num_triangles*3;
        for (int j=0; j<size; j++)
          memcpy(&vis_vertices[vis_len+j], vert[ind[j]].position, sizeof(vec3));
        
        vis_len += size;
      }

      // load textures
      char *tex_types[] = {"spec_", "norm_"};
      if (is_file != NULL) {
        // diffuse
        m->texture = scene_add_texture(scene, tex_name);
      
        // spec
        char spec[strlen(tex_name)+strlen(tex_types[0])];
        strcpy(spec, tex_types[0]);
        strcpy(&spec[strlen(tex_types[0])], tex_name);
        m->texture_spec = scene_add_texture(scene, spec);

        // norm
        char norm[strlen(tex_name)+strlen(tex_types[1])];
        strcpy(norm, tex_types[1]);
        strcpy(&norm[strlen(tex_types[1])], tex_name);
        m->texture_norm = scene_add_texture(scene, norm);
      }

      // push mesh into mesh list
      list_add(model->mesh_list, m);
    }
  }

  // store vertices
  if (keep_vertices) {
    model->vertices = malloc(vis_len*sizeof(vec3));
    model->num_vertices = vis_len;
    memcpy(model->vertices, vis_vertices, vis_len*sizeof(vec3));
  }

  printf("Finished loading IQM model %s\n", path);
  free(vertices);
  free(indices);
  free(data);
  return model;
}