#include "input.h"
#include "engine.h"

uint8_t ex_keys_down[SDL_NUM_SCANCODES];
uint8_t ex_buttons_down[16];

void ex_mouse_callback(SDL_Window* window, double x, double y)
{
  display.mouse_x = x;
  display.mouse_y = y;
}

/*void ex_key_callback(SDL_Window *window, int key, int scancode, int action, int mode)
{
  if (action == EX_PRESS)
    ex_keys_down[key] = 1;
  if (action == EX_RELEASE)
    ex_keys_down[key] = 0;
}

void ex_button_callback(SDL_Window *window, int button, int action, int mods)
{
  if (action == EX_PRESS)
    ex_buttons_down[button] = 1;
  if (action == EX_RELEASE)
    ex_buttons_down[button] = 0;

  if (ex_mousepressed_ptr)
    ex_mousepressed_ptr(button, action, mods);
}*/

void ex_char_callback(SDL_Window *window, unsigned int c)
{
  if (ex_keyinput_ptr)
    ex_keyinput_ptr(c);
}

void ex_scroll_callback(SDL_Window *window, double xoffset, double yoffset)
{
  if (ex_mousescroll_ptr)
    ex_mousescroll_ptr(xoffset, yoffset);
}

void ex_get_mouse_pos(int *x, int *y)
{
  SDL_GetRelativeMouseState((int*)x, (int*)y);
}