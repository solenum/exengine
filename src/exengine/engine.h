#ifndef EX_ENGINE_H
#define EX_ENGINE_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdbool.h>

#include "exe_conf.h"
#include "exengine/window.h"

// user defined function callback pointers
extern void (*ex_init_ptr)(void);
extern void (*ex_update_ptr)(double);
extern void (*ex_draw_ptr)(void);
extern void (*ex_exit_ptr)(void);

// where the magic happens
void exengine();

#endif