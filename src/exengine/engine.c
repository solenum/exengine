#include "engine.h"

// user defined function callback pointers
void (*ex_init_ptr)(void) = NULL;
void (*ex_update_ptr)(double, double) = NULL;
void (*ex_draw_ptr)(void) = NULL;
void (*ex_exit_ptr)(void) = NULL;
// non-essential user callbacks
void (*ex_keypressed_ptr)(uint32_t) = NULL;
void (*ex_mousepressed_ptr)(uint8_t) = NULL;
void (*ex_mousemotion_ptr)(int, int) = NULL;
void (*ex_mousewheel_ptr)(int32_t, int32_t) = NULL;
void (*ex_resize_ptr)(uint32_t, uint32_t) = NULL;
// custom event handling
void (*ex_event_handler)(SDL_Event*) = NULL;
// allows full override of default event handler
void (*ex_event_handler_full)(SDL_Event*) = NULL;

ex_ini_t *conf;

void exengine(char **argv, const char *appname, uint8_t flags)
{
  /* -- INIT ENGINE -- */
  // init physfs filesystem
  PHYSFS_init(argv[0]);
  
  // set the safe writing dir
  // most often these directories will be..
  // linux: ~/.local/share/appname
  // windows: AppData\\Roaming\\appname\\appname
  const char *write_path = PHYSFS_getPrefDir(appname, appname);
  if (write_path != NULL)
    PHYSFS_setWriteDir(write_path);
  else
    printf("PhysFS was unable to set the write directory!\n");

  // append data and write paths to search paths
  PHYSFS_mount(write_path, NULL, 1);
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

  // load engine config file
  conf = malloc(sizeof(ex_ini_t));
  conf->length = 0;
  if (!ex_ini_load(conf, "data/conf.ini")) {
    printf("Failed loading engine config.\n");
  }
  
  // load user config file
  // overwrites any matching variables already loaded
  // from the engine config file
  ex_ini_load(conf, "conf.ini");

  // save the combined user and engine configs
  // to the save location
  ex_ini_save(conf, "conf.ini");

  // load config vars
  uint32_t width = 0, height = 0;
  width = (int)ex_ini_get_float(conf, "graphics", "window_width");
  height = (int)ex_ini_get_float(conf, "graphics", "window_height");
  
  // init the window and gl
  if (!ex_window_init(width, height, "exengine-testing")) {
    ex_exit_ptr();
    return;
  }

  // init rendering modules
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
        // full event handler override
        if (ex_event_handler_full) {
          ex_event_handler_full(&event);
          continue;
        }

        // custom event handler
        // doesnt override anything
        if (ex_event_handler)
          ex_event_handler(&event);

        // default event handler
        switch (event.type) {
          // cya
          case SDL_QUIT:
            running = 0;
            break;

          // input events
          case SDL_KEYDOWN:
          case SDL_KEYUP:
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP:
          case SDL_MOUSEWHEEL:
          case SDL_TEXTEDITING:
          case SDL_TEXTINPUT:
          case SDL_MOUSEMOTION:
          case SDL_KEYMAPCHANGED: {
            ex_input_event(&event);
            break;
          }

          // window events
          case SDL_WINDOWEVENT: {
            ex_window_event(&event);
            break;
          }
        }
      }

      // handle other frequent input updates
      ex_input_update();

      // user update callback
      ex_update_ptr(phys_delta_time, delta_time);

      accumulator -= phys_delta_time;
    }

    
    // user draw callback
    ex_draw_ptr();

    // swap buffers
    SDL_GL_SwapWindow(display.window);
  }
  /* ------------------- */


  // -- CLEAN UP -- */
  free(conf);
  ex_window_destroy();
  PHYSFS_deinit();
  ex_cache_flush();
  if (flags & EX_ENGINE_SOUND)
    ex_sound_exit();

  // user exit callback
  ex_exit_ptr();
  // -------------- */
}