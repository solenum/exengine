/* window
  The window module, handles
  opening a window.  As well
  as window events and input
  callbacks.
*/

#ifndef EX_WINDOW_H
#define EX_WINDOW_H

#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <inttypes.h>

typedef struct {
  SDL_Window *window;
  SDL_GLContext context;
  int width, height;
} ex_window_t;

extern ex_window_t display;

void ex_resize_callback(SDL_Window* window, int width, int height);

/**
 * [ex_window_init creates the window and gl context]
 * @param  width  [window width]
 * @param  height [window height]
 * @param  title  [window title]
 * @return        [true on success]
 */
int ex_window_init(uint32_t width, uint32_t height, const char *title);

/**
 * [ex_window_event window event handler]
 * @param event [SDL_Event pointer]
 */
void ex_window_event(SDL_Event *event);

/**
 * [window_exit clean up any data]
 */
void ex_window_destroy();

#endif // EX_WINDOW_H