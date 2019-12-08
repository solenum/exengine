/* engine
  The core of exengine, inits all internal
  modules, runs the main game loop, and 
  handles all user callbacks.

  It implements a fixed timestep for physics
  to keep things consistent regardless of
  performance and frame rates, while rendering
  at an uncapped framerate assuming vsync is
  not present.

  Feel free to tweak 'phys_delta_time' in engine.c
  to your liking.  Be warned however, physics can
  act up at certain tick rates.  Best to leave
  it at the default if you don't know what you are
  doing.
*/

#ifndef EX_ENGINE_H
#define EX_ENGINE_H

#define EX_DATA_FILE "data.ex"

#include "glad/glad.h"
#include <SDL2/SDL.h>

#include <inttypes.h>
#include <stdbool.h>
#include <physfs.h>
#include <inttypes.h>

#include "render/render.h"
#include "input/input.h"
#include "util/util.h"
#include "sound/sound.h"
#include "math/math.h"

/*
  The module flags, OR these together when
  calling exengine() to enable the modules
  you require.
*/
#define EX_ENGINE_SOUND 1

/*
  User defined function callback pointers, you
  want to set these before you call exengine()
*/
extern void (*ex_init_ptr)(void);
extern void (*ex_update_ptr)(double, double);
extern void (*ex_draw_ptr)(void);
extern void (*ex_exit_ptr)(void);
// non-essential user callbacks
extern void (*ex_keypressed_ptr)(uint32_t);
extern void (*ex_mousepressed_ptr)(uint8_t);
extern void (*ex_mousemotion_ptr)(int, int);
extern void (*ex_mousewheel_ptr)(int32_t, int32_t);
extern void (*ex_resize_ptr)(uint32_t, uint32_t);
// custom event handling
extern void (*ex_event_handler)(SDL_Event*);
// allows full override of default event handler
extern void (*ex_event_handler_full)(SDL_Event*);

/**
 * [exengine starts the internal engine loop]
 * @param argv  []
 * @param appname [custom application name]
 * @param flags [defines what modules to enable]
 */
void exengine(char **argv, const char *appname, uint8_t flags);

#endif // EX_ENGINE_H