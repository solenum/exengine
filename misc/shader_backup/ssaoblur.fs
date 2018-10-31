#version 330 core

out float color;

in vec2 uv;

uniform sampler2D u_ssao;

void main()
{
  vec2 texel_size = 1.0 / vec2(textureSize(u_ssao, 0));
  float result = 0.0;
  for (int x=-2; x<2; x++) {
    for (int y=-2; y<2; y++) {
      vec2 offset = vec2(float(x), float(y)) * texel_size;
      result += texture(u_ssao, uv + offset).r;
    }
  }
  color = result / (4.0 * 4.0);
}