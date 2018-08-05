#include "input.h"
#include "engine.h"

uint8_t ex_keys_down[EX_KEY_LAST];
uint8_t ex_buttons_down[EX_MOUSE_BUTTON_LAST];

void ex_mouse_callback(GLFWwindow* window, double x, double y)
{
  if (!glimgui_focus) {
    display.mouse_x = x;
    display.mouse_y = y;
  }
}

void ex_key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  if (!glimgui_focus) {
    if (action == GLFW_PRESS)
      ex_keys_down[key] = 1;
    if (action == GLFW_RELEASE)
      ex_keys_down[key] = 0;
  } else {
    // pass to imgui
    glimgui_keyinput(key, action);
  }

  if (ex_keypressed_ptr)
    ex_keypressed_ptr(key, scancode, action, mode);
}

void ex_button_callback(GLFWwindow *window, int button, int action, int mods)
{
  if (!glimgui_focus) {
    if (action == GLFW_PRESS)
      ex_buttons_down[button] = 1;
    if (action == GLFW_RELEASE)
      ex_buttons_down[button] = 0;
  } else {
    // pass to imgui
    glimgui_mousebuttoninput(button, action);
  }

  if (ex_mousepressed_ptr)
    ex_mousepressed_ptr(button, action, mods);
}

void ex_char_callback(GLFWwindow *window, unsigned int c)
{
  // pass to imgui
  if (glimgui_focus)
    glimgui_charinput(c);

  if (ex_keyinput_ptr)
    ex_keyinput_ptr(c);
}

void ex_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  // pass to imgui
  if (glimgui_focus)
    glimgui_mousescrollinput(yoffset);

  if (ex_mousescroll_ptr)
    ex_mousescroll_ptr(xoffset, yoffset);
}

void ex_get_mouse_pos(double *x, double *y)
{
  glfwGetCursorPos(display.window, x, y);
}