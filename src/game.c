#include <game.h>
#include <camera.h>
#include <texture.h>
#include <pointlight.h>
#include <scene.h>
#include <list.h>
#include <iqm.h>

float delta_time;
fps_camera_t *camera = NULL;
scene_t *scene;

void game_init()
{
  // load config
  conf_load("data/conf.cfg");

  // load config vars
  uint32_t width = 0, height = 0;
  width = conf_get_int("window_width");
  height = conf_get_int("window_height");
  
  // init the window and gl
  if (!window_init(width, height, "Underwater Zombie Maniac")) {
    game_exit();
    return;
  }

  // init the scene
  scene = scene_new();

  // init the camera
  camera = fps_camera_new(0.0f, 0.0f, 0.0f, 0.03f, 90.0f);
  scene->fps_camera = camera;
}

void game_run()
{ 
  // test iqm model shit
  /*mesh_t *m = iqm_load_model("data/cube.iqm");
  mesh_set_anim(m, 1);
  m->rotation[0] = -90.0f;
  m->position[1] = -7.0f;
  list_add(scene->mesh_list, (void*)m);*/

  mesh_t *m2 = iqm_load_model("data/skelbob.iqm");
  m2->texture = texture_load("data/san.png").id;
  m2->scale = 0.3f;
  m2->rotation[0] = -90.0f;
  m2->position[1] = -6.0f;
  m2->position[0] = 10.0f;
  list_add(scene->mesh_list, (void*)m2);

  mesh_t *m6 = iqm_load_model("data/level.iqm");
  m6->texture = texture_load("data/o.png").id;
  m6->rotation[0] = -90.0f;
  m6->position[1] = -10.0f;
  list_add(scene->mesh_list, (void*)m6);

  double last_frame_time = glfwGetTime();
  while (!glfwWindowShouldClose(display.window)) {
    // handle window events
    glfwPollEvents();

    // calculate delta time
    double current_frame_time = glfwGetTime();
    delta_time = (float)current_frame_time - (float)last_frame_time;
    last_frame_time = current_frame_time;

    //l->position[0] = 15.0f * sin(glfwGetTime());
    //l->position[1] = 3 + 5.0f * cos(glfwGetTime());
    //l->position[2] = 15.0f * cos(glfwGetTime());
    //memcpy(box->position, l->position, sizeof(vec3));
    
    if (keys_down[GLFW_KEY_F]) {
      point_light_t *l = point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, 100.0f);
      memcpy(l->position, camera->position, sizeof(vec3));
      list_add(scene->point_light_list, (void*)l);
      mesh_t *box  = iqm_load_model("data/box.iqm");
      box->texture = texture_load("data/o.png").id;
      box->is_lit  = 0;
      memcpy(box->position, camera->position, sizeof(vec3));
      list_add(scene->mesh_list, (void*)box);
      keys_down[GLFW_KEY_F] = 0;
    }

    /* debug cam movement */
    vec3 speed, side;
    if (keys_down[GLFW_KEY_W]) {
      vec3_scale(speed, camera->front, 0.3f);
      vec3_add(camera->position, camera->position, speed);
    }
    if (keys_down[GLFW_KEY_S]) {
      vec3_scale(speed, camera->front, 0.3f);
      vec3_sub(camera->position, camera->position, speed);
    }
    if (keys_down[GLFW_KEY_A]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, 0.2f);
      vec3_sub(camera->position, camera->position, side);
    }
    if (keys_down[GLFW_KEY_D]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, 0.2f);
      vec3_add(camera->position, camera->position, side);
    }
    if (keys_down[GLFW_KEY_SPACE])
      camera->position[1] += 0.2f;
    if (keys_down[GLFW_KEY_LEFT_CONTROL])
      camera->position[1] -= 0.2f;
    if (keys_down[GLFW_KEY_ESCAPE])
      break;
    /* ------ */

    scene_update(scene, delta_time);
    scene_draw(scene);

    glfwSwapBuffers(display.window);
  }

}

void game_exit()
{
  scene_destroy(scene);
  conf_free();
  window_destroy();
  printf("Exiting\n");
}
