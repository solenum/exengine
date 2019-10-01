#include "engine.h"
#include "render/camera.h"
#include "render/texture.h"
#include "render/pointlight.h"
#include "render/dirlight.h"
#include "render/skybox.h"
#include "render/framebuffer.h"
#include "render/defaults.h"
#include "render/text.h"
#include "input/input.h"
#include "util/exe_list.h"
#include "util/entity.h"
#include "util/scene.h"
#include "util/iqm.h"
#include "util/cache.h"

// renderer feature toggles
int ex_enable_ssao = 1;

// user defined function callback pointers
void (*ex_init_ptr)(void);
void (*ex_update_ptr)(double);
void (*ex_draw_ptr)(void);
void (*ex_exit_ptr)(void);
// non-essential user callbacks
void (*ex_keypressed_ptr)(int);
void (*ex_mousepressed_ptr)(int, int, int);
void (*ex_keyinput_ptr)(unsigned int);
void (*ex_mousescroll_ptr)(double, double);
void (*ex_resize_ptr)(int, int);

conf_t conf;

void exengine(char **argv, uint8_t flags)
{
  /* -- INIT ENGINE -- */
  // init physfs filesystem
  PHYSFS_init(argv[0]);
  printf("%s\n", argv[0]);
  PHYSFS_mount(EX_DATA_FILE, NULL, 1);

  // init engine file data cache
  ex_cache_init();

  // init subsystems
  if (flags & EX_ENGINE_SOUND)
    ex_sound_init();

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

  // init rendering modules
  ex_defaults_textures();
  ex_framebuffer_init();
  ex_font_init();
  
  // user init callback
  ex_init_ptr();
  /* ----------------- */


  /* -- UPDATE ENGINE -- */
  // main engine loop
  double last_ex_frame_time = SDL_GetPerformanceCounter();
  int running = 1;
  SDL_Event e;
  while (running) {
    // calculate delta time
    double current_ex_frame_time = (double)SDL_GetPerformanceCounter();
    delta_time = (double)(current_ex_frame_time - last_ex_frame_time) / (double)SDL_GetPerformanceFrequency();
    last_ex_frame_time = current_ex_frame_time;

    // prevent spiral of death
    if (delta_time > slowest_frame)
      delta_time = slowest_frame;

    // update at a constant rate to keep physics in check
    accumulator += delta_time;
    while (accumulator >= phys_delta_time) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            running = 0;
            break;
          case SDL_KEYDOWN: {
            ex_keys_down[event.key.keysym.scancode] = 1;
            if (event.key.repeat == 0)
              ex_keypressed_ptr(event.key.keysym.scancode);
            break;
          }
          case SDL_KEYUP: {
            ex_keys_down[event.key.keysym.scancode] = 0;
            break;
          }
          case SDL_MOUSEBUTTONDOWN: {
            ex_buttons_down[event.button.button] = 1;
            break;
          }
          case SDL_MOUSEBUTTONUP: {
            ex_buttons_down[event.button.button] = 0;
            break;
          }
        }
      }

      // update mouse coords
      int mx, my;
      SDL_GetRelativeMouseState(&mx, &my);
      display.mouse_x = (float)mx;
      display.mouse_y = (float)my;

      // user update callback
      ex_update_ptr(phys_delta_time);

      accumulator -= phys_delta_time;
    }

    
    // user draw callback
    ex_draw_ptr();

    // swap buffers render gui etc
    ex_window_end();
    SDL_GL_SwapWindow(display.window);
  }
  /* ------------------- */


  // -- CLEAN UP -- */
  conf_free(&conf);
  ex_window_destroy();
  PHYSFS_deinit();
  ex_cache_flush();
  ex_framebuffer_cleanup();
  if (flags & EX_ENGINE_SOUND)
    ex_sound_exit();

  // user exit callback
  ex_exit_ptr();
  // -------------- */
}