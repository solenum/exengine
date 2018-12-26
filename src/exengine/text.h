/* text
 */

#ifndef EX_TEXT_H
#define EX_TEXT_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "msdf.h"

typedef struct {
  GLuint texture;
  GLuint vao, vbo, ebo;
  int modified;
  ex_metrics_t *characters;
  size_t indices[512];
} ex_font_t;

void ex_font_init();

ex_font_t* ex_font_load(const char *path, const char *letters);

#endif // EX_TEXT_H