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

#include "scancodes.h"

extern int ex_mouse_x, ex_mouse_y;
extern uint8_t ex_keys_down[SDL_NUM_SCANCODES];
extern uint8_t ex_buttons_down[16];

/**
 * [ex_input_event handles input events]
 * @param event [SDL_Event pointer]
 */
void ex_input_event(SDL_Event *event);

/**
 * [ex_input_update handles more frequent input updates]
 */
void ex_input_update();

#endif // EX_INPUT_H