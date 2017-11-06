#include "exengine/camera.h"
#include "exengine/texture.h"
#include "exengine/pointlight.h"
#include "exengine/dirlight.h"
#include "exengine/scene.h"
#include "exengine/exe_list.h"
#include "exengine/iqm.h"
#include "exengine/skybox.h"
#include "exengine/entity.h"
#include "exengine/glimgui.h"
#include "exengine/dbgui.h"
#include "inc/game.h"

const double phys_delta_time = 1.0 / 120.0;
const double slowest_frame = 1.0 / 15.0;
double delta_time, accumulator = 0.0;
ex_fps_camera_t *camera = NULL;
ex_scene_t *scene;
conf_t conf;

void game_init()
{
  // load config
  conf_load(&conf, "data/conf.cfg");

  // load config vars
  uint32_t width = 0, height = 0;
  width = conf_get_int(&conf, "window_width");
  height = conf_get_int(&conf, "window_height");
  
  // init the window and gl
  if (!ex_window_init(width, height, "Underwater Zombie Maniac")) {
    game_exit();
    return;
  }

  // init the scene
  scene = scene_new();
  memcpy(scene->gravity, (vec3){0.0f, -0.1f, 0.0f}, sizeof(vec3));

  // init the camera
  camera = ex_fps_camera_new(0.0f, 0.0f, 0.0f, 0.1f, 90.0f);
  scene->fps_camera = camera;
}

void game_run()
{
  ex_model_t *m6 = ex_iqm_load_model(scene, "data/level.iqm", 1);
  list_add(scene->model_list, m6);

  // ex_dir_light_t *dl = ex_dir_light_new((vec3){4.0f, 16.0f, 4.0f}, (vec3){0.4f, 0.4f, 0.45f}, 1);
  // list_add(scene->dir_light_list, dl);

  // ex_skybox_t *s = ex_skybox_new("sky");
  // scene->skybox = s;

  ex_entity_t *e = ex_entity_new(scene, (vec3){0.5f, 1.0f, 0.5f});
  e->position[1] = 1.1f;
  e->position[0] = 1.1f; 
  e->position[2] = 5.0f;
  float move_speed = 1.5f;

  // ex_model_t *d = ex_iqm_load_model(scene, "data/human.iqm", 0);
  // list_add(scene->model_list, d);
  // d->position[1] = 1.5f;
  // d->rotation[0] = -90.0f;
  // ex_model_set_anim(d, 0);

  // ex_model_t *g = ex_iqm_load_model(scene, "data/gun.iqm", 0);
  // list_add(scene->model_list, g);
  // camera->view_model = g;
  // float lastyaw = 0;
  // int aim = 0;
  // vec3 oldpos;
  // ex_model_set_anim(g, 0);
  // camera->view_model_offset[0]  = 0.0f;
  // camera->view_model_offset[1]  = -0.15f;
  // camera->view_model_offset[2]  = 0.25f;

  // ex_model_t *grass = ex_iqm_load_model(scene, "data/tall-grass.iqm", 0);
  // list_add(scene->model_list, grass);
  // grass->position[0] = -2.0f;
  ex_point_light_t *l = ex_point_light_new((vec3){0.0f, 5.0f, 0.0f}, (vec3){0.5f, 0.5f, 0.5f}, 1);
  ex_scene_add_pointlight(scene, l);

  ex_point_light_t *pl = ex_point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.05f, 0.05f, 0.05f}, 0);
  memcpy(pl->position, e->position, sizeof(vec3));
  // ex_scene_add_pointlight(scene, pl);
  pl->is_shadow = 0;

  ex_model_t *box = ex_iqm_load_model(scene, "data/cube.iqm", 0);
  // box->is_shadow = 0;
  list_add(scene->model_list, box);
  ex_entity_t *cube = ex_entity_new(scene, (vec3){1.0f, 1.0f, 1.0f});
  cube->position[1] = 2.5f;

  double last_ex_frame_time = glfwGetTime();
  while (!glfwWindowShouldClose(display.window)) {
    // handle window events
    glfwPollEvents();
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
      ex_entity_update(e, phys_delta_time);
      ex_entity_update(cube, phys_delta_time);

      memcpy(camera->position, e->position, sizeof(vec3));
      camera->position[1] += e->radius[1];
      memcpy(pl->position, camera->position, sizeof(vec3));

      memcpy(box->position, cube->position, sizeof(vec3));

      vec3 temp;
      vec3_sub(temp, cube->position, e->position);
      float len = vec3_len(temp);
      if (len <= cube->radius[1]/2 + e->radius[1]) {
        vec3_norm(temp, temp);
        vec3_scale(temp, temp, len);
        vec3_add(cube->velocity, cube->velocity, temp);
        vec3_sub(e->velocity, e->velocity, temp);
      }

      vec3_scale(temp, cube->velocity, 5.0f * phys_delta_time);
      temp[1] = 0.0f;
      if (cube->grounded == 1)
        vec3_sub(cube->velocity, cube->velocity, temp);

      if (cube->grounded == 0)
        cube->velocity[1] -= (100.0f * phys_delta_time);
      if (cube->velocity[1] <= 0.0f && cube->grounded)
        cube->velocity[1] = 0.0f;

      if (ex_keys_down[GLFW_KEY_LEFT_CONTROL]) {
        vec3 p;
        vec3_scale(p, camera->front, 2.5f);
        vec3_add(p, p, e->position);
        p[1] += 0.5f;
        
        vec3_sub(p, p, cube->position);

        float f = vec3_len(p);
        if (f > 1.5f) {
          ex_keys_down[GLFW_KEY_LEFT_CONTROL] = 0;
          break;
        }

        if (f > 1.0f)
          f *= f;;

        vec3_norm(p, p);
        vec3_scale(p, p, f*35.0f);
        f = cube->velocity[1];
        memcpy(cube->velocity, p, sizeof(vec3));
        cube->velocity[1] += f*0.1f;

        // memset(cube->velocity, 0, sizeof(vec3));
      
        if (ex_buttons_down[GLFW_MOUSE_BUTTON_RIGHT]) {
          vec3_scale(temp, camera->front, 40.0f);
          vec3_add(cube->velocity, cube->velocity, temp);
          ex_keys_down[GLFW_KEY_LEFT_CONTROL] = 0;
        }
      }

      if (ex_keys_down[GLFW_KEY_F]) {
        float r = (float)rand()/(float)(RAND_MAX/1.0f);
        float g = (float)rand()/(float)(RAND_MAX/1.0f);
        float b = (float)rand()/(float)(RAND_MAX/1.0f);
        ex_point_light_t *l = ex_point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){r, g, b}, 1);
        memcpy(l->position, camera->position, sizeof(vec3));
        ex_scene_add_pointlight(scene, l);
        l->is_shadow = 1;
        ex_keys_down[GLFW_KEY_F] = 0;
      }

      /* debug entity movement */
      vec3_scale(temp, e->velocity, 25.0f * phys_delta_time);
      temp[1] = 0.0f;

      if (e->grounded == 1) 
        vec3_sub(e->velocity, e->velocity, temp);
      else
        move_speed = 20.0f;
      
      if (e->grounded == 0)
        e->velocity[1] -= (100.0f * phys_delta_time);
      if (e->velocity[1] <= 0.0f && e->grounded)
        e->velocity[1] = 0.0f;

      if (ex_keys_down[GLFW_KEY_C])
        glfwSwapInterval(1);
      if (ex_keys_down[GLFW_KEY_V])
        glfwSwapInterval(0);

      vec3 speed, side;
      if (ex_keys_down[GLFW_KEY_W]) {
        vec3_norm(speed, (vec3){camera->front[0], 0.0f, camera->front[2]});
        vec3_scale(speed, speed, move_speed * phys_delta_time);
        speed[1] = 0.0f;
        vec3_add(e->velocity, e->velocity, speed);
      }
      if (ex_keys_down[GLFW_KEY_S]) {
        vec3_norm(speed, (vec3){camera->front[0], 0.0f, camera->front[2]});
        vec3_scale(speed, speed, move_speed * phys_delta_time);
        speed[1] = 0.0f;
        vec3_sub(e->velocity, e->velocity, speed);
      }
      if (ex_keys_down[GLFW_KEY_A]) {
        vec3_mul_cross(side, camera->front, camera->up);
        vec3_norm(side, side);
        vec3_scale(side, side, (move_speed*0.9f) * phys_delta_time);
        side[1] = 0.0f;
        vec3_sub(e->velocity, e->velocity, side);
      }
      if (ex_keys_down[GLFW_KEY_D]) {
        vec3_mul_cross(side, camera->front, camera->up);
        vec3_norm(side, side);
        vec3_scale(side, side, (move_speed*0.9f) * phys_delta_time);
        side[1] = 0.0f;
        vec3_add(e->velocity, e->velocity, side);
      }
      if (ex_keys_down[GLFW_KEY_Q])
        e->velocity[1] = 50.0f;
      if (ex_keys_down[GLFW_KEY_Z])
        e->velocity[1] = -50.0f;
      if (ex_keys_down[GLFW_KEY_SPACE] && e->grounded == 1) {
        e->velocity[1] = 20.0f;
      }
      /* if (ex_keys_down[GLFW_KEY_LEFT_CONTROL]) {
        e->radius[1] = 0.5f;
        move_speed = 100.0f;
      } else {
        if (e->radius[1] != 1.0f) {
          e->position[1] += 0.5f;
        }
        e->radius[1] = 1.0f;
      }*/
      move_speed = 200.0f;
      if (ex_keys_down[GLFW_KEY_ESCAPE])
        break;
      if (ex_keys_down[GLFW_KEY_G] || glimgui_ex_keys_down[GLFW_KEY_G]) {
        ex_keys_down[GLFW_KEY_G] = 0;
        glimgui_ex_keys_down[GLFW_KEY_G] = 0;
        glimgui_focus = !glimgui_focus;
      }
      if (ex_keys_down[GLFW_KEY_X]) {
        ex_dbgprofiler.render_octree = !ex_dbgprofiler.render_octree;;
        ex_keys_down[GLFW_KEY_X] = 0;
      }

      memcpy(pl->position, e->position, sizeof(vec3));
      pl->position[1] += 1.0f;
      ex_scene_update(scene, phys_delta_time);

      accumulator -= phys_delta_time;
    }

    ex_scene_draw(scene);
    ex_scene_dbgui(scene);

    /* IMGUI DOCK TEST CRAP
    bool open = 1;
    igBegin("SUP", NULL, 0);
    igBeginDockspace();

    igSetNextDock(ImGuiDockSlot_Left);
    open = igBeginDock("a dock", NULL, 0);
    float f;
    if (open) {
      igSliderFloat("float", &f, 0.0f, 1.0f, "%f", 1.0f);
      igText("Wee");
    }
    igEndDock();

    igSetNextDock(ImGuiDockSlot_Right);
    open = igBeginDock("a pock", NULL, 0);
    if (open) {
      igText("Poo");
      float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
      igPlotLines("wot", arr, 7, 1, NULL, 0.0f, 0.2f, (struct ImVec2){100.0f, 50.0f}, 1);
    }
    igEndDock();

    igSetNextDock(ImGuiDockSlot_Bottom);
    open = igBeginDock("a spock", NULL, 0);
    if (open) {
      igText("Poo");
      igTextColored((struct ImVec4){1.0f, 0.0f, 1.0f, 1.0f}, "PINK");
      igTextColored((struct ImVec4){1.0f, 1.0f, 0.0f, 1.0f}, "YelL0w");
      igTextColored((struct ImVec4){0.5f, 0.8f, 1.0f, 1.0f}, "NeAT");
    }
    igEndDock();

    igDockDebugWindow();

    igEndDockspace();
    igEnd();*/
    
    igShowTestWindow(NULL);
    ex_dbgui_render_profiler();

    ex_window_end();
    glfwSwapBuffers(display.window);
  }
}

void game_exit()
{
  glimgui_shutdown();
  ex_scene_destroy(scene);
  conf_free(&conf);
  ex_window_destroy();
  printf("Exiting\n");
}