#version 330 core

in vec4 frag;

uniform vec3 u_light_pos;
uniform float u_far_plane;
uniform bool u_is_lit;

void main()
{
  if (!u_is_lit) {
    discard;
  }

  float light_distance = length(frag.xyz - u_light_pos);

  light_distance = light_distance / u_far_plane;

  gl_FragDepth = light_distance;
}