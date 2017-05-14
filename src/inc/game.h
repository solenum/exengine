#ifndef GAME_H
#define GAME_H

#include "exengine/exe_conf.h"
#include "exengine/window.h"
#include "exengine/mesh.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdbool.h>

extern float delta_time;

/**
 * [game_init init everything]
 */
void game_init();

/**
 * [game_run the main game loop]
 */
void game_run();

/**
 * [game_exit cleans up any malloc'd data etc]
 */
void game_exit();

#endif // GAME_H