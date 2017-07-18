#ifndef WINDOW_H
#define WINDOW_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <inttypes.h>

#include "glimgui.h"

typedef struct {
	GLFWwindow *window;
  float mouse_x, mouse_y;
} window_t;

extern window_t display;
extern uint8_t keys_down[GLFW_KEY_LAST];
extern uint8_t buttons_down[GLFW_KEY_LAST];

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void button_callback(GLFWwindow *window, int button, int action, int mods);

 void char_callback(GLFWwindow *window, unsigned int c);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void mouse_callback(GLFWwindow* window, double x, double y);

void resize_callback(GLFWwindow* window, int width, int height);

/**
 * [window_init creates the window and gl context]
 * @param  width  [window width]
 * @param  height [window height]
 * @param  title  [window title]
 * @return        [true on success]
 */
bool window_init(uint32_t width, uint32_t height, const char *title);

void window_begin();

void window_end();

/**
 * [window_exit clean up any data]
 */
void window_destroy();

#endif // WINDOW_H