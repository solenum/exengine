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

double delta_time;
fps_camera_t *camera = NULL;
scene_t *scene;
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
  if (!window_init(width, height, "Underwater Zombie Maniac")) {
    game_exit();
    return;
  }

  // init the scene
  scene = scene_new();
  memcpy(scene->gravity, (vec3){0.0f, -0.1f, 0.0f}, sizeof(vec3));

  // init the camera
  camera = fps_camera_new(0.0f, 0.0f, 0.0f, 0.03f, 90.0f);
  scene->fps_camera = camera;
}

void game_run()
{
  model_t *m6 = iqm_load_model(scene, "data/level.iqm", 1);
  list_add(scene->model_list, m6);

  // dir_light_t *dl = dir_light_new((vec3){4.0f, 16.0f, 4.0f}, (vec3){0.4f, 0.4f, 0.45f}, 1);
  // list_add(scene->dir_light_list, dl);

  skybox_t *s = skybox_new("sky");
  scene->skybox = s;

  entity_t *e = entity_new(scene, (vec3){0.5f, 1.0f, 0.5f});
  e->position[1] = 1.1f;
  e->position[0] = 1.1f; 
  e->position[2] = 5.0f;
  float move_speed = 1.5f;

  model_t *d = iqm_load_model(scene, "data/human.iqm", 0);
  list_add(scene->model_list, d);
  // d->position[1] = 1.5f;
  // d->rotation[0] = -90.0f;
  model_set_anim(d, 0);

  model_t *g2 = iqm_load_model(scene, "data/gun.iqm", 0);
  // g2->use_transform = 1;
  g2->position[0] = 2.0f;
  g2->position[1] = 1.0f;
  // list_add(scene->model_list, g2);
  model_set_anim(g2, 0);

  model_t *g = iqm_load_model(scene, "data/gun.iqm", 0);
  // list_add(scene->model_list, g);
  camera->view_model = g;
  float lastyaw = 0;
  int aim = 0;
  vec3 oldpos;
  model_set_anim(g, 0);
  camera->view_model_offset[0]  = 0.0f;
  camera->view_model_offset[1]  = -0.15f;
  camera->view_model_offset[2]  = 0.25f;

  model_t *grass = iqm_load_model(scene, "data/tall-grass.iqm", 0);
  list_add(scene->model_list, grass);
  grass->position[0] = -2.0f;

  point_light_t *pl = point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.2f, 0.2f, 0.25f}, 0);
  memcpy(pl->position, e->position, sizeof(vec3));
  scene_add_pointlight(scene, pl);
  pl->is_shadow = 0;

  double last_frame_time = glfwGetTime();
  while (!glfwWindowShouldClose(display.window)) {
    // handle window events
    glfwPollEvents();
    window_begin();

    // calculate delta time
    double current_frame_time = (double)glfwGetTime();
    delta_time = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;

    entity_update(e, delta_time);
    memcpy(camera->position, e->position, sizeof(vec3));
    camera->position[1] += e->radius[1];
    memcpy(pl->position, camera->position, sizeof(vec3));

    if (keys_down[GLFW_KEY_F]) {
      float r = (float)rand()/(float)(RAND_MAX/1.5f);
      float g = (float)rand()/(float)(RAND_MAX/0.8f);
      float b = (float)rand()/(float)(RAND_MAX/1.5f);
      point_light_t *l = point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){r, g, b}, 0);
      memcpy(l->position, camera->position, sizeof(vec3));
      scene_add_pointlight(scene, l);
      l->is_shadow = 1;
      keys_down[GLFW_KEY_F] = 0;
    }

    /* debug entity movement */
    vec3 temp;
    vec3_scale(temp, e->velocity, 25.0f * delta_time);
    temp[1] = 0.0f;
    
    // can shit
    if (camera->view_model_offset[0] < 0.15f && lastyaw - camera->yaw > 0)
      camera->view_model_offset[0] += ((lastyaw - camera->yaw) * 0.15f) * delta_time;
    if (camera->view_model_offset[0] > 0.05f && lastyaw - camera->yaw < 0)
      camera->view_model_offset[0] += ((lastyaw - camera->yaw) * 0.15f) * delta_time;
    lastyaw = camera->yaw;

    if (e->grounded == 1) 
      vec3_sub(e->velocity, e->velocity, temp);
    else
      move_speed = 20.0f;
    
    if (e->grounded == 0)
      e->velocity[1] -= (100.0f * delta_time);
    else if (e->velocity[1] <= 0.0f)
      e->velocity[1] = 0.0f;

    if (keys_down[GLFW_KEY_C])
      glfwSwapInterval(1);
    if (keys_down[GLFW_KEY_V])
      glfwSwapInterval(0);

    vec3 speed, side;
    if (keys_down[GLFW_KEY_W]) {
      vec3_scale(speed, camera->front, move_speed * delta_time);
      speed[1] = 0.0f;
      vec3_add(e->velocity, e->velocity, speed);
    }
    if (keys_down[GLFW_KEY_S]) {
      vec3_scale(speed, camera->front, move_speed * delta_time);
      speed[1] = 0.0f;
      vec3_sub(e->velocity, e->velocity, speed);
    }
    if (keys_down[GLFW_KEY_A]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, (move_speed*0.9f) * delta_time);
      side[1] = 0.0f;
      vec3_sub(e->velocity, e->velocity, side);
    }
    if (keys_down[GLFW_KEY_D]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, (move_speed*0.9f) * delta_time);
      side[1] = 0.0f;
      vec3_add(e->velocity, e->velocity, side);
    }
    if (keys_down[GLFW_KEY_Q])
      e->velocity[1] = 50.0f;
    if (keys_down[GLFW_KEY_Z])
      e->velocity[1] = -50.0f;
    if (keys_down[GLFW_KEY_SPACE] && e->grounded == 1) {
      e->velocity[1] = 20.0f;
    }
    if (keys_down[GLFW_KEY_LEFT_CONTROL]) {
      e->radius[1] = 0.5f;
      move_speed = 100.0f;
    } else {
      if (e->radius[1] != 1.0f) {
        e->position[1] += 0.5f;
      }
      move_speed = 200.0f;
      e->radius[1] = 1.0f;
    }
    if (keys_down[GLFW_KEY_ESCAPE])
      break;
    if (keys_down[GLFW_KEY_G] || glimgui_keys_down[GLFW_KEY_G]) {
      keys_down[GLFW_KEY_G] = 0;
      glimgui_keys_down[GLFW_KEY_G] = 0;
      glimgui_focus = !glimgui_focus;
    }
    /* ------ */

    memcpy(pl->position, e->position, sizeof(vec3));
    pl->position[1] += 1.0f;
    scene_update(scene, delta_time);
    scene_draw(scene);
    scene_dbgui(scene);
    
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

    window_end();
    glfwSwapBuffers(display.window);
  }
}

void game_exit()
{
  glimgui_shutdown();
  scene_destroy(scene);
  conf_free(&conf);
  window_destroy();
  printf("Exiting\n");
}