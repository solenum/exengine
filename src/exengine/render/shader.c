#include "render/shader.h"
#include <string.h>

#define EX_MAX_SHADERS 512

GLint ex_uniform_map[256][256] = {{0}};
GLint ex_uniform_locations[256][256] = {{0}};

ex_shader_t shader_list[EX_MAX_SHADERS];
size_t shader_count = 0;

GLuint active_shader = 0;

inline GLint ex_uniform(GLuint shader, const char *str)
{
  const char *string = str;
  uint32_t key = 5381;
  int c;

  // hash * 33 + c
  while ((c = *str++))
    key = ((key << 5) + key) + c;

  // check if location cached already
  int i=0;
  for (i=0; i<256; i++) {
    // end of array
    if (!ex_uniform_map[shader][i])
      break;

    // check cached
    if (ex_uniform_map[shader][i] == key)
      return ex_uniform_locations[shader][i];
  }

  // store and return it
  GLint value = glGetUniformLocation(shader, string);
  ex_uniform_map[shader][i] = key;
  ex_uniform_locations[shader][i] = value;

  return value;
}

GLuint ex_shader(const char *path)
{
  // check if shader is already loaded
  for (int i=0; i<EX_MAX_SHADERS; i++)
    if (strcmp(shader_list[i].path, path) == 0)
      return shader_list[i].ID;

  // prefix path with shader dir
  char real_path[256];
  ex_io_prefix_str(real_path, path, EX_SHADER_LOC);

  char *str = ex_io_read(real_path, "r", NULL);
  char *shaders[3] = {NULL, NULL, NULL};
  const char *types[][2] = {
    {"#START VS", "#END VS"},
    {"#START FS", "#END FS"},
    {"#START GS", "#END GS"}
  };

  // extract shaders
  for (int i=0; i<3; i++) {
    char *start = strstr(str, types[i][0]);
    char *end   = strstr(str, types[i][1]);
    if (start && end) {
      size_t len = (end - start)  - 10;
      shaders[i] = malloc(len);
      strncpy(shaders[i], &start[10], len);
      shaders[i][len-1] = '\0';
    }    
  }

  // create the shaders
  GLuint vertex_shader, fragment_shader, geometry_shader;
  vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);

  // compile the shaders
  glShaderSource(vertex_shader, 1, (const GLchar* const*)&shaders[0], NULL);
  glCompileShader(vertex_shader);

  GLint success = 0;
  GLchar compile_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, compile_log);
    printf("Failed to compile vertex shader\n%s\n", compile_log);
    goto exit;
  }

  glShaderSource(fragment_shader, 1, (const GLchar* const*)&shaders[1], NULL);
  glCompileShader(fragment_shader);

  success = 0;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, compile_log);
    printf("Failed to compile fragment shader\n%s\n", compile_log); 
    goto exit;
  }

  if (shaders[2] != NULL) {
    glShaderSource(geometry_shader, 1, (const GLchar* const*)&shaders[2], NULL);
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
  if (shaders[2] != NULL)
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

  for (int i=0; i<3; i++) {
    if (shaders[i] != NULL)
      free(shaders[i]);
  }

  printf("Shaders (%s) successfully compiled\n", path);

  shader_list[shader_count].ID = shader_program;
  strcpy(shader_list[shader_count].path, path);
  if (shader_count < EX_MAX_SHADERS)
    shader_count++;
  else
    printf("Max number of shaders reached!\n");

  return shader_program;
}

void ex_shader_use(GLuint shader)
{
  if (active_shader == shader)
    return;

  glUseProgram(shader);
}