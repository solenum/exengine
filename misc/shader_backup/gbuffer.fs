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