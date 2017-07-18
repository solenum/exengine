#include "glimgui.h"
#include "shader.h"

static GLFWwindow   *window;
static double       time = 0.0f;
static bool         mousepressed[3] = { false, false, false };
static float        mousewheel = 0.0f;
static GLuint       fonttexture = 0;
static int          shader = 0, locationtex = 0, locationproj = 0;
static int          locationposition = 0, locationUV = 0, locationcolor = 0;
static GLuint       VBO = 0, VAO = 0, EAO = 0;
bool glimgui_focus = 0;
uint8_t glimgui_keys_down[GLFW_KEY_LAST];

void glimgui_init(GLFWwindow *win)
{
  window = win;
  struct ImGuiIO *io = igGetIO();
  io->KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  io->KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  io->KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io->KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io->KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io->KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io->KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
  io->KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
  io->KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  io->KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  io->KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  io->KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  io->KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  io->KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
  io->KeyMap[ImGuiKey_A] = GLFW_KEY_A;
  io->KeyMap[ImGuiKey_C] = GLFW_KEY_C;
  io->KeyMap[ImGuiKey_V] = GLFW_KEY_V;
  io->KeyMap[ImGuiKey_X] = GLFW_KEY_X;
  io->KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
  io->KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

  io->ClipboardUserData = win;
#ifdef _WIN32
  io->ImeWindowHandle = glfwGetWin32Window(window);
#endif

  io->RenderDrawListsFn = glimgui_renderdrawlists;
}

void glimgui_renderdrawlists(struct ImDrawData *draw_data)
{
  struct ImGuiIO *io = igGetIO();
  int fb_width  = (int)(io->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height = (int)(io->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width == 0 || fb_height == 0)
    return;

  // backup GL state
  GLint last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
  glActiveTexture(GL_TEXTURE0);
  GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
  GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
  GLint last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, &last_blend_src_rgb);
  GLint last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, &last_blend_dst_rgb);
  GLint last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, &last_blend_src_alpha);
  GLint last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, &last_blend_dst_alpha);
  GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
  GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
  GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
  GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

  // setup render state
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);

  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  const float ortho_projection[4][4] = {
      { 2.0f/io->DisplaySize.x, 0.0f,                   0.0f, 0.0f },
      { 0.0f,                  2.0f/-io->DisplaySize.y, 0.0f, 0.0f },
      { 0.0f,                  0.0f,                   -1.0f, 0.0f },
      {-1.0f,                  1.0f,                    0.0f, 1.0f },
  };
  glUseProgram(shader);
  glUniform1i(locationtex, 0);
  glUniformMatrix4fv(locationproj, 1, GL_FALSE, &ortho_projection[0][0]);
  glBindVertexArray(VAO);

  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    struct ImDrawList *cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx *idx_buffer_offset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)ImDrawList_GetVertexBufferSize(cmd_list) * sizeof(ImDrawVert), (const GLvoid*)ImDrawList_GetVertexPtr(cmd_list, 0), GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EAO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)ImDrawList_GetIndexBufferSize(cmd_list) * sizeof(ImDrawIdx), (const GLvoid*)ImDrawList_GetIndexPtr(cmd_list, 0), GL_STREAM_DRAW);

    for (int cmd_i = 0; cmd_i < ImDrawList_GetCmdSize(cmd_list); cmd_i++) {
      const struct ImDrawCmd *pcmd = ImDrawList_GetCmdPtr(cmd_list, cmd_i);
    
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
        glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }

  // restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
  if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
  if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
  if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
  if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

void glimgui_createfontstexture()
{
  // build texture atlas
  struct ImGuiIO *io = igGetIO();
  unsigned char *pixels;
  int width, height, n;
  ImFontAtlas_GetTexDataAsRGBA32(io->Fonts, &pixels, &width, &height, &n);

  // upload texture
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &fonttexture);
  glBindTexture(GL_TEXTURE_2D, fonttexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  ImFontAtlas_SetTexID(igGetIO()->Fonts, (void *)(intptr_t)fonttexture);

  glBindTexture(GL_TEXTURE_2D, last_texture);
}

void glimgui_createobjects()
{
  GLint last_texture, last_array_buffer, last_vertex_array;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

  shader = shader_compile("data/imgui.vs", "data/imgui.fs", NULL);

  locationtex = glGetUniformLocation(shader, "Texture");
  locationproj = glGetUniformLocation(shader, "ProjMtx");
  locationposition = glGetAttribLocation(shader, "Position");
  locationUV = glGetAttribLocation(shader, "UV");
  locationcolor = glGetAttribLocation(shader, "Color");

  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EAO);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glEnableVertexAttribArray(locationposition);
  glEnableVertexAttribArray(locationUV);
  glEnableVertexAttribArray(locationcolor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
  glVertexAttribPointer(locationposition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
  glVertexAttribPointer(locationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
  glVertexAttribPointer(locationcolor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

  glimgui_createfontstexture();

  // Restore modified GL state
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindVertexArray(last_vertex_array);
}

void glimgui_invalidateobjects()
{
  if (VAO)
    glDeleteVertexArrays(1, &VAO);
  
  if (VBO)
    glDeleteBuffers(1, &VBO);

  if (EAO)
    glDeleteBuffers(1, &EAO);

  VAO = VBO = EAO = 0;

  if (shader)
    glDeleteProgram(shader);
  shader = 0;

  if (fonttexture) {
    glDeleteTextures(1, &fonttexture);
    ImFontAtlas_SetTexID(igGetIO()->Fonts, 0);
    fonttexture = 0;
  }
}

void glimgui_shutdown()
{
  glimgui_invalidateobjects();
  igShutdown();
}

void glimgui_newframe()
{
  if (!fonttexture)
    glimgui_createobjects();

  struct ImGuiIO *io = igGetIO();

  // set frame size
  int w, h, display_w, display_h;
  glfwGetWindowSize(window, &w, &h);
  glfwGetFramebufferSize(window, &display_w, &display_h);
  io->DisplaySize.x = (float)w;
  io->DisplaySize.y = (float)h;
  io->DisplayFramebufferScale.x = w > 0 ? ((float)display_w / w) : 0;
  io->DisplayFramebufferScale.y = h > 0 ? ((float)display_h / h) : 0;

  // time step
  double current_time = glfwGetTime();
  io->DeltaTime = time > 0.0 ? (float)(current_time - time) : (float)(1.0f/60.0f);
  time = current_time;

  // setup inputs
  if (glfwGetWindowAttrib(window, GLFW_FOCUSED) && glimgui_focus) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    io->MousePos.x = (float)mx;
    io->MousePos.y = (float)my;
  } else {
    io->MousePos.x = -1;
    io->MousePos.y = -1;
  }

  for (int i=0; i<3; i++) {
    if (glimgui_focus) {
      io->MouseDown[i] = mousepressed[i] || glfwGetMouseButton(window, i) != 0;
      mousepressed[i]  = 0;
    }
  }

  io->MouseWheel = mousewheel;
  mousewheel = 0.0f;

  // hide mouse if need be
  // glfwSetInputMode(window, GLFW_CURSOR, io->MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

  igNewFrame();
}

static const char* glimgui_getclipboard(void *user_data)
{
  return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void glimgui_setclipboard(void *user_data, const char *text)
{
  glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void glimgui_mousebuttoninput(int button, int action)
{
  if (action == GLFW_PRESS && button >= 0 && button < 23)
    mousepressed[button] = 1;
}

void glimgui_mousescrollinput(double offset)
{
  mousewheel += (float)offset;
}

void glimgui_keyinput(int key, int action)
{
  struct ImGuiIO *io = igGetIO();

  if (action == GLFW_PRESS) {
    glimgui_keys_down[key] = 1;
    io->KeysDown[key] = 1;
  }
  if (action == GLFW_RELEASE) {
    glimgui_keys_down[key] = 0;
    io->KeysDown[key] = 0;
  }

  // modifier keys
  io->KeyCtrl  = io->KeysDown[GLFW_KEY_LEFT_CONTROL] || io->KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io->KeyShift = io->KeysDown[GLFW_KEY_LEFT_SHIFT] || io->KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io->KeyAlt   = io->KeysDown[GLFW_KEY_LEFT_ALT] || io->KeysDown[GLFW_KEY_RIGHT_ALT];
  io->KeySuper = io->KeysDown[GLFW_KEY_LEFT_SUPER] || io->KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void glimgui_charinput(unsigned int c)
{
  struct ImGuiIO *io = igGetIO();
  if (c > 0 && c < 0x10000)
    ImGuiIO_AddInputCharacter((unsigned short)c);
}