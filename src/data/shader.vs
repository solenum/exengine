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

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_bone_matrix[200];

void main() 
{
  mat4 skeleton = u_bone_matrix[int(in_boneindex.x)] * in_boneweights.x +
                  u_bone_matrix[int(in_boneindex.y)] * in_boneweights.y +
                  u_bone_matrix[int(in_boneindex.z)] * in_boneweights.z +
                  u_bone_matrix[int(in_boneindex.w)] * in_boneweights.w;

  mat4 transform = u_model * skeleton;

  gl_Position   = u_projection * u_view * transform * vec4(in_position, 1.0); 
  normal = mat3(transpose(inverse(transform))) * in_normals;
  frag          = vec3(transform * vec4(in_position, 1.0f));
  uv            = in_uv;
  color         = in_color;
}