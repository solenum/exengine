/* text
  ttf loading, msdf generation and
  text rendering
 */

#ifndef EX_TEXT_H
#define EX_TEXT_H

#define GLEW_STATIC
#include <GL/glew.h>

typedef struct {
  GLuint texture;
} ex_font_t;

ex_font_t* ex_font_load(const char *path); 

#endif // EX_TEXT_H