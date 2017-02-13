#ifndef WINDOW_H
#define WINDOW_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <camera.h>
#include <game.h>

#include <inttypes.h>
#include <stdbool.h>
#include <shader.h>

typedef struct {
	GLFWwindow *window;
  GLuint shader_program;
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
 * [window_init_shaders loads and compiles shaders]
 * @param  vertex_path   [vertex shader file path]
 * @param  fragment_path [fragment shader file path]
 */
void window_init_shaders(const char *vertex_path, const char *fragment_path);

/**
 * [window_exit clean up any data]
 */
void window_destroy();

#endif // WINDOW_H