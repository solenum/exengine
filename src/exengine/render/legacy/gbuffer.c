#include "render/gbuffer.h"
#include "render/window.h"
#include "render/shader.h"
#include "render/ssao.h"

GLuint gbuffer, gposition, gnormal, gcolorspec, grenderbuffer;
GLuint gvao, gvbo;
GLuint ex_gshader, ex_gmainshader;
int width, height;

void ex_gbuffer_init(int reinit)
{
  glGenFramebuffers(1, &gbuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);

  SDL_GetWindowSize(display.window, &width, &height);

  // position buffer
  glGenTextures(1, &gposition);
  glBindTexture(GL_TEXTURE_2D, gposition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gposition, 0);

  // normal buffer
  glGenTextures(1, &gnormal);
  glBindTexture(GL_TEXTURE_2D, gnormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10_A2, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gnormal, 0);

  // color + spec (albedo)
  glGenTextures(1, &gcolorspec);
  glBindTexture(GL_TEXTURE_2D, gcolorspec);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gcolorspec, 0);

  // set buffers for rendering
  GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments);

  // generate render buffer
  glGenRenderbuffers(1, &grenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, grenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, grenderbuffer);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Error! Framebuffer is not complete in gbuffer\n");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (!reinit) {
    // compile shaders
    ex_gshader     = ex_shader("gbuffer.glsl");
    ex_gmainshader = ex_shader("gmain.glsl");

    /* -- screen quad -- */
    GLfloat vertices[] = {   
      // pos         // uv
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
      -1.0f,  1.0f,  0.0f, 1.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
       1.0f,  1.0f,  1.0f, 1.0f
    };

    // vao for framebuffer
    glGenVertexArrays(1, &gvao);
    glGenBuffers(1, &gvbo);
    glBindVertexArray(gvao);

    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, gvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // tex coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, (GLvoid*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    /* ----------------- */
  }
}

void ex_gbuffer_first(int x, int y, int vw, int vh)
{
  glUseProgram(ex_gshader);
  glViewport(x, y, vw, vh);
  glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);
}

void ex_gbuffer_render(GLuint shader)
{
  // bind textures
  glUniform1i(ex_uniform(shader, "u_position"), 0);
  glUniform1i(ex_uniform(shader, "u_norm"), 1);
  glUniform1i(ex_uniform(shader, "u_colorspec"), 2);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gposition);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gnormal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gcolorspec);

  glUniform1i(ex_uniform(shader, "u_point_depth"), 4);

  // draw quad
  glBindVertexArray(gvao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void ex_gbuffer_resize(int width, int height)
{
  ex_gbuffer_destroy();
  ex_gbuffer_init(1);
}

void ex_gbuffer_destroy()
{
  glDeleteRenderbuffers(1, &grenderbuffer);
  glDeleteFramebuffers(1, &gbuffer);
  glDeleteTextures(1, &gposition);
  glDeleteTextures(1, &gnormal);
  glDeleteTextures(1, &gcolorspec);
}