#include <stdio.h>
#include "exengine/engine.h"
#include "inc/game.h"

int main(int argc, char **argv)
{
  ex_init_ptr = game_init;
  ex_update_ptr = game_update;
  ex_draw_ptr = game_draw;
  ex_exit_ptr = game_exit;

  // set to 0 to disable ssao
  ex_enable_ssao = 1;

  exengine(argv);

  return EXIT_SUCCESS;
}