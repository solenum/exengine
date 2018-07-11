#include "defaults.h"

GLuint default_texture_diffuse, default_texture_normal, default_texture_specular;
GLuint default_texture_ssao;

char diffuse_data[] = {0,0,0,255};
char normal_data[] = {128,127,255,255};
char specular_data[] = {0,0,0,255};
char ssao_data[] = {255,255,255,255};

GLuint gen_texture(const char *data, GLuint type)
{
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, type, 1, 1, 0, type, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}

void ex_defaults_textures()
{
  // default mesh textures
  default_texture_diffuse = gen_texture(diffuse_data, GL_RGBA);
  default_texture_normal = gen_texture(normal_data, GL_RGBA);
  default_texture_specular = gen_texture(specular_data, GL_RGBA);
  default_texture_ssao = gen_texture(ssao_data, GL_RED);
}