#START VS
#version 330 core

layout (location = 0) in vec3 in_position;

uniform mat4 u_view;
uniform mat4 u_projection;

void main() 
{
  gl_Position = u_projection * u_view * vec4(in_position, 1.0f);
}
#END VS

#START FS
#version 330 core

out vec4 color;

void main()
{
  color = vec4(1.0f, 0.1f, 0.1f, 1.0f);
}
#END FS