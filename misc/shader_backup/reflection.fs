#version 330 core

layout (location = 0) out vec3 color;

in vec4 ffrag;
in vec2 fuv;

uniform sampler2D u_texture;
uniform vec3 u_probe_pos;

void main()
{
  color = texture(u_texture, fuv).rgb;
  // gl_FragDepth = length(ffrag.xyz - u_probe_pos) / 75.0;
}