#version 330 core

layout (location = 0) in vec3 in_position;

out vec3 uv;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_view_position;

void main()
{
  // discard translation
  mat4 view = u_view;
  view[3][0] = 0.0f;
  view[3][1] = 0.0f;
  view[3][2] = 0.0f;
  
  gl_Position = u_projection * view * vec4(in_position, 1.0);
  uv = in_position;
}