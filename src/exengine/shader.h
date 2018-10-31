/* shader
  Loads and compiles shaders.

  Requires at minimal a vertex and
  fragment shader, can also compile a
  geometry shader if specified.
*/

#ifndef EX_SHADER_H
#define EX_SHADER_H

#define EX_SHADER_LOC "data/shaders/"

#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include "exe_io.h"

/**
 * [ex_uniform cache and return shader uniform locations]
 * @param  shader [shader to be used]
 * @param  str    [uniform string]
 * @return        [uniform location]
 *
 * hashes a string key using djb2
 * www.cse.yorku.ca/~oz/hash.html
 */
GLint ex_uniform(GLuint shader, const char *str);

/**
 * [ex_shader_compile loads, attaches and links shaders into a shader program]
 * @param  path   [shader file path]
 * @return        [the shader program GLuint]
 *
 * Move this to a .c file, it shouldn't be
 * in the header.
 */

GLuint ex_shader_compile(const char *path);


#endif // EX_SHADER_H
