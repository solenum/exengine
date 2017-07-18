#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

texture_t* texture_load(const char *file, int get_data)
{
  // prepend file directory
  size_t len = strlen(TEXTURE_LOC);
  char file_dir[len + strlen(file)];
  strcpy(file_dir, TEXTURE_LOC);
  strcpy(&file_dir[len], file);
  
  printf("Loading texture %s\n", file_dir);

  // attempt to load image
  int w,h,n;
  uint8_t *data = stbi_load(file_dir, &w, &h, &n, 4);
  if (data == NULL) {
    printf("Could not load texture %s\n", file_dir);
    return NULL;
  }

  // create texture obj
  texture_t *t = malloc(sizeof(texture_t));
  t->width  = w;
  t->height = h;
  strncpy(t->name, file, 32);
  
  // do we want the data?
  if (get_data == 1) {
    // we force 4 attributes 
    size_t size = (w*h)*4;
    
    // copy image data
    t->data = malloc(size);
    memcpy(t->data, data, size);
    
    // free stbi data
    stbi_image_free(data);
    
    return t;
  }

  // create a gl texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // set some basic params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  float aniso = 0.0f;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // unset texture
  glBindTexture(GL_TEXTURE_2D, 0);
  
  t->id = texture;

  // clean up data
  stbi_image_free(data);

  return t;
}