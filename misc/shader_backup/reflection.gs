#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_shadow_matrices[6];

in vec2 uv[];

out vec4 ffrag;
out vec2 fuv;

void main()
{
  for (int face=0; face<6; ++face) {
    gl_Layer = face;
    for (int i=0; i<3; ++i) {
      ffrag = gl_in[i].gl_Position;
      fuv = uv[i];
      gl_Position = u_shadow_matrices[face] * ffrag;
      EmitVertex();
    }
    EndPrimitive();
  }
}