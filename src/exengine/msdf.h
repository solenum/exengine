/* msdf
  Handles multi-channel signed distance field bitmap
  generation from given ttf (stb_truetype.h) font.

  Based on the C++ implementation by Viktor Chlumský.
  https://github.com/Chlumsky/msdfgen
*/

#ifndef EX_MSDF_H
#define EX_MSDF_H

#include "stb_truetype.h"

typedef struct {
  double dist;
  double d;
} signed_distance_t;

// the possible types:
// STBTT_vmove  = start of a contour
// STBTT_vline  = linear segment
// STBTT_vcurve = quadratic segment
// STBTT_vcubic = cubic segment
typedef struct {
  int color;
  vec2 p[4];
  int type;
} edge_segment_t;

// defines what color channel an edge belongs to
typedef enum {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7
} edge_color_t;

float* ex_msdf_glyph(stbtt_fontinfo *font, uint8_t c, size_t w, size_t h);

#endif // EX_MSDF_H