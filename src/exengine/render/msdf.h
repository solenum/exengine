/* msdf
  Handles multi-channel signed distance field bitmap
  generation from given ttf (stb_truetype.h) font.

  Depends on stb_truetype.h to load the ttf file.

  This is in an unstable state, ymmv.

  Based on the C++ implementation by Viktor Chlumský.
  https://github.com/Chlumsky/msdfgen

  You probably dont want to use this directly.
  Instead, use text.h
*/

#ifndef EX_MSDF_H
#define EX_MSDF_H

#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "render/stb_truetype.h"

typedef struct {
  int left_bearing;
  int advance;
  int ix0, ix1;
  int iy0, iy1;
  int tx,  ty;
  uint32_t character;
} ex_metrics_t;

/**
 * [ex_msdf_glyph Generates a MSDF bitmap from the specified character]
 * @param  font    [font file, ttf or otf]
 * @param  c       [utf8 glyph to use]
 * @param  w       [bitmap width]
 * @param  h       [bitmap height]
 * @param  metrics [the metrics for the specified glyph (c)]
 * @return         [3-channel float array, remember to free this]
 */
float* ex_msdf_glyph(stbtt_fontinfo *font, uint32_t c, size_t w, size_t h, ex_metrics_t *metrics);

static inline uint32_t ex_utf8(const char *c) {
  uint32_t val = 0;

  if ((c[0] & 0xF8) == 0xF0) {
    // 4 byte
    val |= (c[3] & 0x3F);
    val |= (c[2] & 0x3F) << 6;
    val |= (c[1] & 0x3F) << 12;
    val |= (c[0] & 0x07) << 18;
  }
  else if ((c[0] & 0xF0) == 0xE0) {
    // 3 byte
    val |= (c[2] & 0x3F);
    val |= (c[1] & 0x3F) << 6;
    val |= (c[0] & 0x0F) << 12;
  }
  else if ((c[0] & 0xE0) == 0xC0) {
    // 2 byte
    val |= (c[1] & 0x3F);
    val |= (c[0] & 0x1F) << 6;
  }
  else {
    // 1 byte
    val = c[0];
  }

  return val;
}

#endif // EX_MSDF_H