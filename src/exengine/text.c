#include "text.h"
#include "exe_io.h"
#include "mathlib.h"
#include "msdf.h"
#include <string.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define INF   -1e24
#define RANGE 1.0

ex_font_t* ex_font_load(const char *path)
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

  // generate a msdf bitmap
  // this is all just debug vomit
  // its due to be replaced 冥
  int size = 128;
  float *bitmap = ex_msdf_glyph(&font, '.', size, size);
  uint8_t *shitmap = malloc(3*size*size);
  memset(shitmap, 0, 3*size*size);
  for (int y=0; y<size; y++) {
    for (int x=0; x<size; x++) {
      size_t index = 3*((y*size)+x);

      // im certain this isnt how you math
      float v = median(bitmap[index], bitmap[index+1], bitmap[index+2]) - 0.5;
      v *= vec2_mul_inner((vec2){2.0f/size, 2.0f/size}, (vec2){x/0.5, y/0.5});
      float a = MAX(0.0, MIN(v + 0.5, 1.0));
      a = sqrt(1.0 * 1.0 * (1.0 - a) + 0.0 * 0.0 * a);

      // reeeeeeeeeeee
      shitmap[index]   = 255*a;//(uint8_t)(255*(bitmap[index]+64)/64);
      shitmap[index+1] = 255*a;//(uint8_t)(255*(bitmap[index+1]+64)/64);
      shitmap[index+2] = 255*a;//(uint8_t)(255*(bitmap[index+2]+64)/64);
    }
  }

  // debug output
  stbi_write_png("out.png", size, size, 3, shitmap, size*3);

  free(data);
  free(bitmap);
  free(shitmap);
  printf("[TEXT] Done generating msdf font for %s\n", path);

  // ex_font_t *f = malloc(sizeof(ex_font_t));
  return NULL;
}