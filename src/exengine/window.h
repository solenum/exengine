#ifndef WINDOW_H
#define WINDOW_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdbool.h>

typedef struct {
	GLFWwindow *window;
  float mouse_x, mouse_y;
} window_t;

extern window_t display;
extern uint8_t keys_down[GLFW_KEY_LAST];

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
  if (action == GLFW_PRESS)
    keys_down[key] = 1;
  if (action == GLFW_RELEASE)
    keys_down[key] = 0;
}

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

/**
 * [window_exit clean up any data]
 */
void window_destroy();

#endif // WINDOW_H