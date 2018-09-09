#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D u_texture;

vec3 aces_tonemap(vec3 x) {
  float a = 2.51;
  float b = 0.03;
  float c = 2.43;
  float d = 0.59;
  float e = 0.14;
  return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

const vec3 u_white_point = vec3(0.75, 0.75, 0.75);

void main()
{
  vec3 tex_color = textureLod(u_texture, uv, 0.0).rgb;
  if (length(tex_color) <= 0)
    discard;
  tex_color = vec3(1.0) - exp(-tex_color / u_white_point);
  color = vec4(aces_tonemap(tex_color), 1.0);
}