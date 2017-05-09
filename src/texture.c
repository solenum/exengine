#include <texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

texture_t texture_load(const char *file)
{
  printf("Loading texture %s\n", file);

  // attempt to load image
  int w,h,n;
  uint8_t *data = stbi_load(file, &w, &h, &n, 4);
  if (data == NULL) {
    printf("Could not load texture %s\n", file);
  }

  // create a gl texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // set some basic params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  // unset texture and clean up data
  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);

  texture_t t;
  t.id      = texture;
  t.width   = w;
  t.height  = h;

  return t;
}