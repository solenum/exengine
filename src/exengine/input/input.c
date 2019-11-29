#include "input/input.h"
#include "engine.h"

int ex_mouse_x = 0, ex_mouse_y = 0;
uint8_t ex_keys_down[EX_NUM_SCANCODES];
uint8_t ex_buttons_down[16];

void ex_input_event(SDL_Event *event)
{
  switch (event->type) {
    // keyboard
    case SDL_KEYDOWN: {
      ex_keys_down[event->key.keysym.scancode] = 1;
      if (event->key.repeat == 0 && ex_keypressed_ptr)
        ex_keypressed_ptr(event->key.keysym.scancode);
      break;
    }
    case SDL_KEYUP: {
      ex_keys_down[event->key.keysym.scancode] = 0;
      break;
    }

    // mouse
    case SDL_MOUSEBUTTONDOWN: {
      ex_buttons_down[event->button.button] = 1;
      if (event->button.state == SDL_PRESSED && ex_keypressed_ptr) 
        ex_mousepressed_ptr(event->button.button);
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      ex_buttons_down[event->button.button] = 0;
      break;
    }
    case SDL_MOUSEWHEEL: {
      if (ex_mousewheel_ptr)
        ex_mousewheel_ptr(event->wheel.x, event->wheel.y);
      break;
    }
    case SDL_MOUSEMOTION: {
      if (ex_mousemotion_ptr)
        ex_mousemotion_ptr(event->motion.xrel, event->motion.yrel);
      break;
    }
  }
}

void ex_input_update()
{
  if (SDL_GetRelativeMouseMode())
    SDL_GetRelativeMouseState(&ex_mouse_x, &ex_mouse_y);
  else
    SDL_GetMouseState(&ex_mouse_x, &ex_mouse_y);
}