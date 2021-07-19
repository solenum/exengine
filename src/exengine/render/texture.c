#include "render/texture.h"
#include <physfs.h>

#define STB_IMAGE_IMPLEMENTATION
#include "render/stb_image.h"

ex_texture_t* ex_texture_load(const char *file_name, int get_data)
{
  // prepend file directory
  size_t len = strlen(EX_TEXTURE_LOC);
  char file_dir[len + strlen(file_name)];
  strcpy(file_dir, EX_TEXTURE_LOC);
  strcpy(&file_dir[len], file_name);

  printf("Loading texture %s\n", file_dir);

  if (!PHYSFS_exists(file_dir)) {
    printf("Texture does not exist %s\n", file_dir);
    return NULL;
  }

  // load the file
  PHYSFS_file *file;
  file = PHYSFS_openRead(file_dir);
  size_t size = PHYSFS_fileLength(file);

  // read into buffer
  char *buff = malloc(size+1);
  PHYSFS_readBytes(file, buff, size);
  buff[size] = '\0';
  PHYSFS_close(file);

  // attempt to load image
  int w,h,n;
  uint8_t *data = stbi_load_from_memory((uint8_t*)buff, size, &w, &h, &n, 4);
  if (data == NULL) {
    printf("Could not load texture %s\n", file_dir);
    return NULL;
  }

  // create texture obj
  ex_texture_t *t = malloc(sizeof(ex_texture_t));
  t->width  = w;
  t->height = h;
  strncpy(t->name, file_name, 32);
  
  // do we want the data?
  if (get_data == 1) {
    // we force 4 attributes
    size_t size = (w*h)*4;
    
    // copy image data
    t->data = malloc(size);
    memcpy(t->data, data, size);
    
    // free stbi data
    stbi_image_free(data);
    free(buff);
    
    return t;
  }

  // create a gl texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // set some basic params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // float aniso = 0.0f;
  // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // unset texture
  glBindTexture(GL_TEXTURE_2D, 0);
  
  t->id = texture;

  // clean up data
  stbi_image_free(data);
  free(buff);

  return t;
}