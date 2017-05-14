#include <stdio.h>
#include "inc/game.h"

int main(int argc, char **argv)
{
  game_init();

  game_run();

  game_exit();

  return EXIT_SUCCESS;
}