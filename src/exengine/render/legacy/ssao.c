#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "render/ssao.h"
#include "render/shader.h"
#include "render/framebuffer.h"
#include "render/defaults.h"
#include "util/ini.h"

extern ex_ini_t *conf;
extern GLuint gposition, gnormal;
extern GLuint fbo_vao;

vec3 ssao_samples[SSAO_NUM_SAMPLES];
vec3 ssao_noise[16];

// ssao geometry pass
GLuint ssao_noise_texture, ssao_fbo, ssao_color_buffer;
GLuint ssao_shader;
GLuint sample_loc = 0, projection_loc = 0, view_loc = 0, screensize_loc = 0;
GLuint gposition_loc = 0, gnormal_loc = 0, noise_loc = 0;

// ssao blur pass
GLuint ssao_blur_fbo, ssao_color_blur_buffer;
GLuint ssao_blur_shader;
GLuint ssao_blur_loc = 0;

void ssao_init()
{
  srand(time(NULL));

  // generate kernel sample hemispheres
  for (int i=0; i<SSAO_NUM_SAMPLES; i++) {
    float r1 = (float)rand()/(float)(RAND_MAX/1.0);
    float r2 = (float)rand()/(float)(RAND_MAX/1.0);
    float r3 = (float)rand()/(float)(RAND_MAX/1.0);
    float r4 = (float)rand()/(float)(RAND_MAX/1.0);

    vec3 sample = {r1 * 2.0 - 1.0, r2 * 2.0 - 1.0, r3};
    vec3_norm(sample, sample);
    sample[0] *= r4;
    sample[1] *= r4;
    sample[2] *= r4;

    // scale samples to be closer to the center
    float scale = (float)i / (float)SSAO_NUM_SAMPLES;
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample[0] *= scale;
    sample[1] *= scale;
    sample[2] *= scale;

    memcpy(ssao_samples[i], sample, sizeof(vec3));
  }

  // generate kernel noise
  for (int i=0; i<16; i++) {
    float r1 = (float)rand()/(float)(RAND_MAX/1.0);
    float r2 = (float)rand()/(float)(RAND_MAX/1.0);
  
    vec3 noise = {r1 * 2.0 - 1.0, r2 * 2.0 - 1.0, 0.0f};
    memcpy(ssao_noise[i], noise, sizeof(vec3));
  }

  // generate a texture to hold the noise values
  glGenTextures(1, &ssao_noise_texture);
  glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // generate the ssao framebuffer
  glGenFramebuffers(1, &ssao_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);

  int width = (int)ex_ini_get_float(conf, "graphics", "window_width");
  int height = (int)ex_ini_get_float(conf, "graphics", "window_height");

  // generate color buffer
  glGenTextures(1, &ssao_color_buffer);
  glBindTexture(GL_TEXTURE_2D, ssao_color_buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_color_buffer, 0);

  // test framebuffer
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Error! SSAO Framebuffer is not complete\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // generate the ssao blur framebuffer
  glGenFramebuffers(1, &ssao_blur_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);

  // generate ssao blur color buffer
  glGenTextures(1, &ssao_color_blur_buffer);
  glBindTexture(GL_TEXTURE_2D, ssao_color_blur_buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_color_blur_buffer, 0);

  // test blur framebuffer
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Error! SSAO Blur Framebuffer is not complete\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // load and init the shaders
  ssao_shader = ex_shader("ssao.glsl");
  ssao_blur_shader = ex_shader("ssao.glsl");
}

void ssao_render(mat4x4 projection, mat4x4 view)
{
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(ssao_shader);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gposition);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gnormal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);

  if (!sample_loc)
    sample_loc = ex_uniform(ssao_shader, "u_samples");
  if (!projection_loc)
    projection_loc = ex_uniform(ssao_shader, "u_projection");
  if (!view_loc)
    view_loc = ex_uniform(ssao_shader, "u_view");
  if (!screensize_loc)
    screensize_loc = ex_uniform(ssao_shader, "u_screensize");
  if (!gposition_loc)
    gposition_loc = ex_uniform(ssao_shader, "u_gposition");
  if (!gnormal_loc)
    gnormal_loc = ex_uniform(ssao_shader, "u_gnormal");
  if (!noise_loc)
    noise_loc = ex_uniform(ssao_shader, "u_noise");

  int width = (int)ex_ini_get_float(conf, "graphics", "window_width");
  int height = (int)ex_ini_get_float(conf, "graphics", "window_height");
  vec2 screensize = {width, height};
  
  glUniform2fv(screensize_loc, 1, (float*)&screensize[0]);
  glUniform3fv(sample_loc, SSAO_NUM_SAMPLES, (float*)&ssao_samples[0]);
  glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (float*)&projection[0]);
  glUniformMatrix4fv(view_loc, 1, GL_FALSE, (float*)&view[0]);
  glUniform1i(gposition_loc, 0);
  glUniform1i(gnormal_loc, 1);
  glUniform1i(noise_loc, 2);

  // render screen quad
  glBindVertexArray(fbo_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // do blur pass
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(ssao_blur_shader);

  // send the ssao color texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ssao_color_buffer);
  if (!ssao_blur_loc)
    ssao_blur_loc = ex_uniform(ssao_blur_shader, "u_ssao");
  glUniform1i(ssao_blur_loc, 0);

  glBindVertexArray(fbo_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  
  glBindVertexArray(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ssao_bind_texture(GLuint shader)
{
  glActiveTexture(GL_TEXTURE3);
  glUniform1i(ex_uniform(shader, "u_ssao"), 3);
  glBindTexture(GL_TEXTURE_2D, ssao_color_blur_buffer);
}

void ssao_bind_default(GLuint shader)
{
  glActiveTexture(GL_TEXTURE3);
  glUniform1i(ex_uniform(shader, "u_ssao"), 3);
  glBindTexture(GL_TEXTURE_2D, default_texture_ssao); 
}