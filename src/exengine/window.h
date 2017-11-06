#ifndef EX_WINDOW_H
#define EX_WINDOW_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <inttypes.h>

#include "glimgui.h"

typedef struct {
	GLFWwindow *window;
  float mouse_x, mouse_y;
} ex_window_t;

extern ex_window_t display;
extern uint8_t ex_keys_down[GLFW_KEY_LAST];
extern uint8_t ex_buttons_down[GLFW_KEY_LAST];

void ex_key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void ex_button_callback(GLFWwindow *window, int button, int action, int mods);

void ex_char_callback(GLFWwindow *window, unsigned int c);

void ex_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void ex_mouse_callback(GLFWwindow* window, double x, double y);

void ex_resize_callback(GLFWwindow* window, int width, int height);

/**
 * [ex_window_init creates the window and gl context]
 * @param  width  [window width]
 * @param  height [window height]
 * @param  title  [window title]
 * @return        [true on success]
 */
bool ex_window_init(uint32_t width, uint32_t height, const char *title);

void ex_window_begin();

void ex_window_end();

/**
 * [window_exit clean up any data]
 */
void ex_window_destroy();

#endif // EX_WINDOW_H