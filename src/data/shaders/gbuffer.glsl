#START VS
#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normals;
layout (location = 3) in vec4 in_tangents;
layout (location = 4) in vec4 in_color;
layout (location = 5) in vec4 in_boneindex;
layout (location = 6) in vec4 in_boneweights;
layout (location = 7) in mat4 in_instancematrix;

out vec3 frag;
out vec3 normal;
out vec2 uv;
out mat3 TBN;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_bone_matrix[200];
uniform bool u_has_skeleton;

void main()
{
  mat4 transform = in_instancematrix;
  if (u_has_skeleton == true) {
    vec4 boneindex = in_boneindex*255.0;
    vec4 boneweights = in_boneweights*255.0;
    mat4 skeleton = u_bone_matrix[int(boneindex.x)] * boneweights.x +
                    u_bone_matrix[int(boneindex.y)] * boneweights.y +
                    u_bone_matrix[int(boneindex.z)] * boneweights.z +
                    u_bone_matrix[int(boneindex.w)] * boneweights.w;

    transform = transform * skeleton;
  }

  vec4 v = u_projection * u_view * transform * vec4(in_position, 1.0);

  gl_Position    = v;
  normal         = mat3(transpose(inverse(u_view * transform))) * in_normals;
  frag           = vec3(u_view * in_instancematrix * vec4(in_position, 1.0f));
  uv             = in_uv;

  // calculate tbn matrix for normal mapping
  vec3 T = normalize(vec3(u_view * transform * vec4(in_tangents.xyz, 0.0)));
  vec3 B = normalize(vec3(u_view * transform * in_tangents));
  vec3 N = normalize(vec3(u_view * transform * vec4(in_normals, 0.0)));
  TBN    = mat3(T, B, N);
}
#END VS


#START FS
#version 330 core

layout (location = 0) out vec3 Position;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec4 Colorspec;

in vec3 frag;
in vec3 normal;
in vec2 uv;
in mat3 TBN;

uniform sampler2D u_texture;
uniform sampler2D u_spec;
uniform sampler2D u_norm;

void main()
{  
  Position = frag;

  vec3 norm = normalize(normal);

  norm = texture(u_norm, uv).rgb;
  norm = normalize(norm * 2.0 - 1.0);
  norm = normalize(TBN * norm);

  Normal = norm * 0.5 + 0.5;
  
  Colorspec.rgb = texture(u_texture, uv).rgb;
  Colorspec.a = texture(u_spec, uv).r;
}
#END FS