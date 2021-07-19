#version 330 core

uniform bool u_is_lit;

void main()
{
  if (!u_is_lit) {
    discard;
  }

  gl_FragDepth = gl_FragCoord.z;
}