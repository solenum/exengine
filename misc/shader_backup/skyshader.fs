#version 330 core

in vec3 uv;

out vec4 color;

uniform samplerCube u_skybox;

void main()
{
  color = texture(u_skybox, uv);
}