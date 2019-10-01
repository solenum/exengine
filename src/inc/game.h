#ifndef GAME_H
#define GAME_H

#include "glad/glad.h"
#include <SDL2/SDL.h>

#include <inttypes.h>
#include <stdbool.h>

/**
 * [game_init init everything]
 */
void game_init();

/**
 * [game_update the main game loop]
 */
void game_update(double dt);

/**
 * [game_draw]
 */
void game_draw();

/**
 * [game_exit cleans up any malloc'd data etc]
 */
void game_exit();

void game_keypressed(int key);

void game_resize(int width, int height);

#endif // GAME_H