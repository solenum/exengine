#version 330 core

layout (location = 0) out vec3 Position;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec4 Colorspec;

in vec3 frag;
in vec3 normal;
in vec2 uv;
in vec4 color;
in vec4 frag_light_pos;
in float fog;
in mat3 TBN;

uniform sampler2D u_texture;
uniform sampler2D u_spec;
uniform sampler2D u_norm;
uniform bool u_dont_norm;

void main()
{  
  Position = frag;

  vec3 norm = normalize(normal);

  if (!u_dont_norm) {
    norm = texture(u_norm, uv).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    norm = normalize(TBN * norm);
  }

  Normal = norm * 0.5 + 0.5;
  
  Colorspec.rgb = texture(u_texture, uv).rgb;
  Colorspec.a = texture(u_spec, uv).r;
}