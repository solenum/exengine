#include "window.h"
#include <stdio.h>

window_t display;
uint8_t keys_down[GLFW_KEY_LAST];
uint8_t buttons_down[GLFW_KEY_LAST];

bool window_init(uint32_t width, uint32_t height, const char *title)
{
  // init glfw
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
  
  // create glfw window
  display.window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (display.window == NULL) {
    printf("Failed to initialize window\n");
    glfwTerminate();
    return 0;
  }

  // set callbacks
  glfwSetKeyCallback(display.window, key_callback);
  glfwSetCursorPosCallback(display.window, mouse_callback);
  glfwSetFramebufferSizeCallback(display.window, resize_callback);
  glfwSetMouseButtonCallback(display.window, button_callback);
  glfwSetScrollCallback(display.window, scroll_callback);
  glfwSetCharCallback(display.window, char_callback);

  // set context
  glfwMakeContextCurrent(display.window);

  // init glew
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("Error initializing glew\n%s\n", glewGetErrorString(err));
    return 0;
  }
  
  // set viewport etc
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glCullFace(GL_BACK);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_FRAMEBUFFER_SRGB);

  glfwSetInputMode(display.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // setup imgui debug gui
  glimgui_init(display.window);

  return 1;
}

void window_begin()
{
  if (!glimgui_focus)
    glfwSetInputMode(display.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  else
    glfwSetInputMode(display.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  glimgui_newframe();
}

void window_end()
{
  igRender();
}

void window_destroy()
{
  glfwTerminate();
}

void resize_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double x, double y)
{
  if (!glimgui_focus) {
    display.mouse_x = x;
    display.mouse_y = y;
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  if (!glimgui_focus) {
    if (action == GLFW_PRESS)
      keys_down[key] = 1;
    if (action == GLFW_RELEASE)
      keys_down[key] = 0;
  } else {
    // pass to imgui
    glimgui_keyinput(key, action);
  }
}

void button_callback(GLFWwindow *window, int button, int action, int mods)
{
  if (!glimgui_focus) {
    if (action == GLFW_PRESS)
      buttons_down[button] = 1;
    if (action == GLFW_RELEASE)
      buttons_down[button] = 0;
  } else {
    // pass to imgui
    glimgui_mousebuttoninput(button, action);
  }
}

void char_callback(GLFWwindow *window, unsigned int c)
{
  // pass to imgui
  if (glimgui_focus)
    glimgui_charinput(c);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  // pass to imgui
  if (glimgui_focus)
    glimgui_mousescrollinput(yoffset);
}