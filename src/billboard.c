#include <game.h>
#include <texture.h>
#include <billboard.h>
#include <map.h>
#include <stdio.h>
#include <stdlib.h>

billboard_t *billboard_new(GLuint shader_program, texture_t texture, float width, float height, float scale)
{
  // create the billboard
  billboard_t *b    = malloc(sizeof(billboard_t));
  b->shader_program = shader_program;
  
  b->width  = width;
  b->height = height;
  b->scale  = scale/2;
  
  b->frame        = 0;
  b->frame_start  = 0;
  b->frame_end    = 0;
  b->frame_timer  = 0;
  b->frame_speed  = 0;
  b->texture      = texture;

  // a front-facing quad
  float uv_offset_x = width/texture.width;
  float uv_offset_y = height/texture.height;
  float size = scale/2;
  GLfloat vertices[] = {
    -scale,  -scale, scale,  0.0f, 0.0f, 1.0f, 0.0f,        uv_offset_y,
     scale,  -scale, scale,  0.0f, 0.0f, 1.0f, uv_offset_x, uv_offset_y,
     scale,   scale, scale,  0.0f, 0.0f, 1.0f, uv_offset_x, 0.0f,
     scale,   scale, scale,  0.0f, 0.0f, 1.0f, uv_offset_x, 0.0f,
    -scale,   scale, scale,  0.0f, 0.0f, 1.0f, 0.0f,        0.0f,
    -scale,  -scale, scale,  0.0f, 0.0f, 1.0f, 0.0f,        uv_offset_y
  };
  GLuint indices[] = {
    0,1,2,3,4,5
  };

  // create the mesh
  b->mesh = mesh_new(vertices, sizeof vertices / sizeof vertices[0], indices, sizeof indices / sizeof indices[0], texture.id);

  return b;
}

void billboard_animate(billboard_t *b)
{
  if (b->frame_speed > 0.0)
    b->frame_timer += delta_time;
  
  if (b->frame_timer > b->frame_speed) {
    b->frame_timer = 0.0;
    b->frame++;
  }

  if (b->frame > b->frame_end)
    b->frame = b->frame_start;
}

void billboard_draw(billboard_t *b)
{
  // set billboarding
  GLuint isbillboard_location = glGetUniformLocation(b->shader_program, "uni_is_billboard");
  glUniform1i(isbillboard_location, 1);

  // set animating
  GLuint animate_location = glGetUniformLocation(b->shader_program, "uni_is_animated");
  if (b->frame_speed > 0.0) {
    glUniform1i(animate_location, 1);
  
    // uv offset for animation
    GLuint uv_location = glGetUniformLocation(b->shader_program, "uni_uv_offset");
    int tile_count = b->texture.width/b->width;
    int tx = (b->frame-((b->frame/tile_count)*tile_count));
    int ty = (b->frame/tile_count);
    glUniform2f(uv_location, (b->width/b->texture.width)*tx, (b->height/b->texture.height)*ty);
  }

  // frame number
  GLuint frame_location = glGetUniformLocation(b->shader_program, "uni_frame");
  glUniform1i(frame_location, b->frame);
  
  // render it
  mesh_draw(b->mesh, b->shader_program);
  
  // unset billboarding
  glUniform1i(isbillboard_location, 0);
  glUniform1i(animate_location, 0);
}

void billboard_set_position(billboard_t *b, float x, float y, float z)
{
  mat4x4_translate(b->mesh->transform, x, y, z);
}

void billboard_destroy(billboard_t *b)
{
  free(b->mesh);
  free(b);
  b = NULL;
}