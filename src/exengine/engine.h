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

#define EX_ENGINE_SOUND 1

// user defined function callback pointers
extern void (*ex_init_ptr)(void);
extern void (*ex_update_ptr)(double);
extern void (*ex_draw_ptr)(void);
extern void (*ex_exit_ptr)(void);
extern void (*ex_keypressed_ptr)(int, int, int, int);
extern void (*ex_mousepressed_ptr)(int, int, int);
extern void (*ex_keyinput_ptr)(unsigned int);
extern void (*ex_mousescroll_ptr)(double, double);

// where the magic happens
void exengine(char **argv, uint8_t flags);

#endif // EX_ENGINE_H