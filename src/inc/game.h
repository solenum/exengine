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
void game_update(double dt, double ft);

/**
 * [game_draw]
 */
void game_draw();

/**
 * [game_exit cleans up any malloc'd data etc]
 */
void game_exit();

void game_keypressed(uint32_t key);

void game_mousepressed(uint8_t button);

void game_mousemoition(int xrel, int yrel);

void game_mousewheel(int32_t x, int32_t y);

void game_resize(uint32_t width, uint32_t height);

#endif // GAME_H