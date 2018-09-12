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

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdbool.h>
#include <physfs.h>
#include <inttypes.h>

#include "exe_conf.h"
#include "exengine/window.h"
#include "exengine/sound.h"

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
extern void (*ex_update_ptr)(double);
extern void (*ex_draw_ptr)(void);
extern void (*ex_exit_ptr)(void);
extern void (*ex_keypressed_ptr)(int, int, int, int);
extern void (*ex_mousepressed_ptr)(int, int, int);
extern void (*ex_keyinput_ptr)(unsigned int);
extern void (*ex_mousescroll_ptr)(double, double);
extern void (*ex_resize_ptr)(int, int);

/**
 * [exengine starts the internal engine loop]
 * @param argv  []
 * @param flags [defines what modules to enable]
 */
void exengine(char **argv, uint8_t flags);

#endif // EX_ENGINE_H