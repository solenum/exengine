#include <stdio.h>
#include <exengine/engine.h>
#include "inc/sdk.h"

int main(int argc, char **argv)
{
  ex_init_ptr = sdk_init;
  ex_update_ptr = sdk_update;
  ex_draw_ptr = sdk_draw;
  ex_exit_ptr = sdk_exit;

  exengine();

  return EXIT_SUCCESS;
}