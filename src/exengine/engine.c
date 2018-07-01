#include "engine.h"
#include "camera.h"
#include "texture.h"
#include "pointlight.h"
#include "dirlight.h"
#include "scene.h"
#include "exe_list.h"
#include "iqm.h"
#include "skybox.h"
#include "entity.h"
#include "glimgui.h"
#include "dbgui.h"

// user defined function callback pointers
void (*ex_init_ptr)(void);
void (*ex_update_ptr)(double);
void (*ex_draw_ptr)(void);
void (*ex_exit_ptr)(void);

conf_t conf;

void exengine()
{
  /* -- INIT ENGINE -- */
  // delta time vars
  const double phys_delta_time = 1.0 / 60.0;
  const double slowest_frame = 1.0 / 15.0;
  double delta_time, accumulator = 0.0;

  // load user defined config
  conf_load(&conf, "data/conf.cfg");

  // load config vars
  uint32_t width = 0, height = 0;
  width = conf_get_int(&conf, "window_width");
  height = conf_get_int(&conf, "window_height");
  
  // init the window and gl
  if (!ex_window_init(width, height, "exengine-testing")) {
    ex_exit_ptr();
    return;
  }
  
  // user init callback
  ex_init_ptr();
  /* ----------------- */


  /* -- UPDATE ENGINE -- */
  // main engine loop
  double last_ex_frame_time = glfwGetTime();
  while (!glfwWindowShouldClose(display.window)) {
    // handle window events
    ex_window_begin();

    // calculate delta time
    double current_ex_frame_time = (double)glfwGetTime();
    delta_time = current_ex_frame_time - last_ex_frame_time;
    last_ex_frame_time = current_ex_frame_time;

    // prevent spiral of death
    if (delta_time > slowest_frame)
      delta_time = slowest_frame;

    // update at a constant rate to keep physics in check
    accumulator += delta_time;
    while (accumulator >= phys_delta_time) {
      glfwPollEvents();

      // user update callback
      ex_update_ptr(phys_delta_time);

      accumulator -= phys_delta_time;
    }

    
    // user draw callback
    ex_draw_ptr();

    // swap buffers render gui etc
    ex_window_end();
    glfwSwapBuffers(display.window);
  }
  /* ------------------- */


  // -- CLEAN UP -- */
  glimgui_shutdown();
  conf_free(&conf);
  ex_window_destroy();

  // user exit callback
  ex_exit_ptr();
  // -------------- */
}