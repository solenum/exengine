#include "window.h"
#include "input.h"
#include <stdio.h>

ex_window_t display;

bool ex_window_init(uint32_t width, uint32_t height, const char *title)
{
  // init glfw
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // create glfw window
  display.window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (display.window == NULL) {
    printf("Failed to initialize window\n");
    glfwTerminate();
    return 0;
  }

  // set callbacks
  glfwSetKeyCallback(display.window, ex_key_callback);
  glfwSetCursorPosCallback(display.window, ex_mouse_callback);
  glfwSetFramebufferSizeCallback(display.window, ex_resize_callback);
  glfwSetMouseButtonCallback(display.window, ex_button_callback);
  glfwSetScrollCallback(display.window, ex_scroll_callback);
  glfwSetCharCallback(display.window, ex_char_callback);

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
  glfwSwapInterval(0);
  glfwSetInputMode(display.window, GLFW_STICKY_KEYS, 1);

  // setup imgui debug gui
  glimgui_init(display.window);

  return 1;
}

void ex_window_begin()
{
  if (!glimgui_focus)
    glfwSetInputMode(display.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  else
    glfwSetInputMode(display.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  glimgui_newframe();
}

void ex_window_end()
{
  igRender();
}

void ex_window_destroy()
{
  glfwTerminate();
}

void ex_resize_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}
