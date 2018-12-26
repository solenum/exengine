#include "text.h"
#include "exe_io.h"
#include "mathlib.h"
#include "shader.h"
#include <string.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define INF   -1e24
#define RANGE 1.0
#define SIZE  16

GLuint shader;

void ex_font_init()
{
  shader = ex_shader_compile("text.glsl");
}

ex_font_t* ex_font_load(const char *path, const char *letters)
{
  // load the ttf data
  size_t length;
  uint8_t *data = (uint8_t*)io_read_file(path, "rb", &length);
  if (!data) {
    printf("[TEXT] Failed generating font %s\n", path);
    return NULL;
  }

  // process the ttf data
  stbtt_fontinfo font;
  stbtt_InitFont(&font, (const uint8_t*)data, stbtt_GetFontOffsetForIndex(data,0));

  size_t atlas_width = strlen(letters)*SIZE;
  float *atlas = malloc(sizeof(float)*(strlen(letters)*SIZE*SIZE)*3);

  ex_metrics_t metrics;
  char *character = (char*)letters;
  size_t index = 0;
  while (*character != '\0') {
    char c = *character++;
    float *bitmap = ex_msdf_glyph(&font, ex_utf8(&c), SIZE, SIZE, &metrics);

    memcpy(&atlas[index*SIZE*SIZE*3], bitmap, sizeof(float)*SIZE*SIZE*3);
    index++;

    free(bitmap);
  }

  ex_font_t *f = malloc(sizeof(ex_font_t));

  glGenTextures(1, &f->texture);
  glBindTexture(GL_TEXTURE_2D, f->texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atlas_width, atlas_width, 0, GL_RGB, GL_FLOAT, atlas);

  glBindTexture(GL_TEXTURE_2D, 0);

  free(data);
  free(atlas);
  printf("[TEXT] Done generating msdf font for %s\n", path);

  return f;
}