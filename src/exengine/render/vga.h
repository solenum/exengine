/* vga
  Simple vga style font rendering specifically
  for debug information, not for production use.
*/

#ifndef EX_VGA_H
#define EX_VGA_H

#include <inttypes.h>
#include <stdlib.h>
#include "glad/glad.h"

extern uint32_t ex_vga_fg;
extern uint32_t ex_vga_bg;

/**
 * [ex_vga_init init vga systems]
 */
void ex_vga_init();

/**
 * [ex_vga_print blit some text to the vga]
 * @param x   [x index]
 * @param y   [y index]
 * @param str [ascii string]
 */
void ex_vga_print(size_t x, size_t y, const char *str);

/**
 * [ex_vga_render render vga to the screen]
 */
void ex_vga_render();

/**
 * [ex_vga_clear clear the vga pixel data]
 */
void ex_vga_clear();

/**
 * [ex_vga_setfg set the foreground color]
 * @param r [red]
 * @param g [green]
 * @param b [blue]
 * @param a [alpha]
 */
void ex_vga_setfg(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * [ex_vga_setbg set the background color]
 * @param r [red]
 * @param g [green]
 * @param b [blue]
 * @param a [alpha]
 */
void ex_vga_setbg(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * [ex_vga_destroy clean up vga systems]
 */
void ex_vga_destroy();

#endif // EX_VGA_H