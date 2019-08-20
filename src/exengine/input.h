/* input
  Handles all user input, such as
  keyboard, mouse, gamepad, etc.

  Key codes will work for every keyboard
  layout.  They work by mapping each key
  of a US keyboard layout to the closest
  key on whatever layout the user has.

  For example, Q is the top-left character
  key for all keyboard layouts.

  Don't manually use the callbacks here,
  instead set the engine function pointers
  and your functions will be called when
  an event fires.  See engine.h for details.
*/

#ifndef EX_INPUT_H
#define EX_INPUT_H

#include <inttypes.h>
#include <SDL2/SDL.h>

extern uint8_t ex_keys_down[SDL_NUM_SCANCODES];
extern uint8_t ex_buttons_down[16];

/**
 * [ex_key_callback handled internally]
 * @param window   [glfw window]
 * @param key      []
 * @param scancode []
 * @param action   []
 * @param mode     []
 */
void ex_key_callback(SDL_Window *window, int key, int scancode, int action, int mode);

/**
 * [ex_button_callback handled internally]
 * @param window []
 * @param button []
 * @param action []
 * @param mods   []
 */
void ex_button_callback(SDL_Window *window, int button, int action, int mods);

/**
 * [ex_char_callback handled internally]
 * @param window []
 * @param c      []
 */
void ex_char_callback(SDL_Window *window, unsigned int c);

/**
 * [ex_scroll_callback handled internally]
 * @param window  []
 * @param xoffset []
 * @param yoffset []
 */
void ex_scroll_callback(SDL_Window *window, double xoffset, double yoffset);

/**
 * [ex_mouse_callback handled internally]
 * @param window []
 * @param x      []
 * @param y      []
 */
void ex_mouse_callback(SDL_Window* window, double x, double y);

/**
 * [ex_get_mouse_pos relative mouse position to window]
 * @param x [x offset]
 * @param y [y offset]
 */
void ex_get_mouse_pos(int *x, int *y);

#endif // EX_INPUT_H