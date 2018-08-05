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
 * @param  vertex_path   [vertex shader file path]
 * @param  fragment_path [fragment shader file path]
 * @return               [the shader program GLuint]
 */
static GLuint ex_shader_compile(const char *vertex_path, const char *fragment_path, const char *geometry_path)
{
  char vpath[256], fpath[256], gpath[256];
  io_prefix_str(vpath, vertex_path, EX_SHADER_LOC);
  io_prefix_str(fpath, fragment_path, EX_SHADER_LOC);
  io_prefix_str(gpath, geometry_path, EX_SHADER_LOC);

	printf("Loading shader files %s, %s and %s\n", vpath, fpath, gpath);

	// load shader files
	char *vertex_source = NULL, *fragment_source = NULL, *geometry_source = NULL;
	vertex_source = io_read_file(vpath, "r", NULL);
  fragment_source = io_read_file(fpath, "r", NULL);
  if (vertex_source == NULL || fragment_source == NULL) {
    printf("Failed creating shader\n");
    
    // clean up
    if (vertex_source != NULL)
      free(vertex_source);
    if (fragment_source != NULL)
      free(fragment_source);
    
    return 0;
  }

  if (strlen(gpath) > 0) {
    geometry_source = io_read_file(gpath, "r", NULL);

    if (geometry_source == NULL) {
      printf("Failed creating geometry shader\n");

      // clean up
      if (vertex_source != NULL)
        free(vertex_source);
      if (fragment_source != NULL)
        free(fragment_source);
      if (geometry_source != NULL)
        free(geometry_source);
      
      return 0;      
    }
  }

  // create the shaders
  GLuint vertex_shader, fragment_shader, geometry_shader;
  vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);

  // compile the shaders
  glShaderSource(vertex_shader, 1, (const GLchar**)&vertex_source, NULL);
  glCompileShader(vertex_shader);

  GLint success = 0;
  GLchar compile_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, compile_log);
    printf("Failed to compile vertex shader\n%s\n", compile_log);
    goto exit;
  }

  glShaderSource(fragment_shader, 1, (const GLchar**)&fragment_source, NULL);
  glCompileShader(fragment_shader);

  success = 0;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, compile_log);
    printf("Failed to compile fragment shader\n%s\n", compile_log); 
    goto exit;
  }

  if (geometry_source != NULL) {
    glShaderSource(geometry_shader, 1, (const GLchar**)&geometry_source, NULL);
    glCompileShader(geometry_shader);

    success = 0;
    glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(geometry_shader, 512, NULL, compile_log);
      printf("Failed to compile geometry shader\n%s\n", compile_log); 
      goto exit;
    }
  }

	// create shader program
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  if (geometry_source != NULL)
	 glAttachShader(shader_program, geometry_shader);
	glLinkProgram(shader_program);

	success = 0;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, compile_log);
		printf("Failed to link shader program\n%s\n", compile_log);
		goto exit;
	}

exit:
	glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
	glDeleteShader(geometry_shader);

  if (vertex_source != NULL)
    free(vertex_source);
  if (fragment_source != NULL)
    free(fragment_source);
  if (geometry_source != NULL)
    free(geometry_source);

	printf("Shaders successfully compiled\n");

	return shader_program;
}


#endif // EX_SHADER_H
