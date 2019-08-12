/* window
  The window module, handles
  opening a window.  As well
  as window events and input
  callbacks.
*/

#ifndef EX_WINDOW_H
#define EX_WINDOW_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <inttypes.h>

#include "glimgui.h"

typedef struct {
	GLFWwindow *window;
  int width, height;
  float mouse_x, mouse_y;
} ex_window_t;

extern ex_window_t display;

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