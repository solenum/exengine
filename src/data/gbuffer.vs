#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normals;
layout (location = 3) in vec4 in_tangents;
layout (location = 4) in vec4 in_color;
layout (location = 5) in vec4 in_boneindex;
layout (location = 6) in vec4 in_boneweights;

out vec3 frag;
out vec3 normal;
out vec2 uv;
out vec4 color;
out vec4 frag_light_pos;
out float fog;
out mat3 TBN;

uniform mat4 u_model; 
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_bone_matrix[200];
uniform bool u_has_skeleton;
uniform bool u_is_viewmodel;
uniform mat4 u_light_transform;

void main()
{
  mat4 transform = u_model;
  if (u_has_skeleton == true) {
    vec4 boneindex = in_boneindex*255.0;
    vec4 boneweights = in_boneweights*255.0;
    mat4 skeleton = u_bone_matrix[int(boneindex.x)] * boneweights.x +
                    u_bone_matrix[int(boneindex.y)] * boneweights.y +
                    u_bone_matrix[int(boneindex.z)] * boneweights.z +
                    u_bone_matrix[int(boneindex.w)] * boneweights.w;

    transform = u_model * skeleton;
  }

  vec4 v = u_projection * u_view * transform * vec4(in_position, 1.0);

  gl_Position    = v;
  normal         = mat3(transpose(inverse(u_view * transform))) * in_normals;
  frag           = vec3(u_view * u_model * vec4(in_position, 1.0f));
  uv             = in_uv;
  color          = in_color;
  frag_light_pos = u_light_transform * vec4(frag, 1.0);

  // calculate tbn matrix for normal mapping
  vec3 T = normalize(vec3(u_view * transform * vec4(in_tangents.xyz, 0.0)));
  vec3 B = normalize(vec3(u_view * transform * in_tangents));
  vec3 N = normalize(vec3(u_view * transform * vec4(in_normals, 0.0)));
  TBN    = mat3(T, B, N);
}