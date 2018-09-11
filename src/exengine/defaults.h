/* default
  Defines defaults for textures and other
  essential data.
*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

#define GLEW_STATIC
#include <GL/glew.h>

extern GLuint default_texture_diffuse;
extern GLuint default_texture_normal;
extern GLuint default_texture_specular;
extern GLuint default_texture_ssao;

/**
 * [ex_defaults_textures generate the default textures]
 */
void ex_defaults_textures();

#endif // DEFAULTS_H