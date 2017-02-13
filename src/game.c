#include <game.h>
#include <camera.h>
#include <texture.h>
#include <iqm.h>

float delta_time;
iso_camera_t *camera;
texture_t tiles_texture, entities_texture;

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

  // load and compile shaders
  window_init_shaders("data/shader.vs", "data/shader.fs");

  // init the camera
  camera = iso_camera_new(0.0f, 0.0f, 0.0f, 0.03f, 75.0f);

  // load textures
  tiles_texture     = texture_load("data/tiles.png");
  entities_texture  = texture_load("data/entities_1.png");
}

void game_run()
{
  // test iqm model
  mesh_t *m = iqm_load_model("data/cube.iqm");
  mat4x4_rotate(m->transform, m->transform, 1.0f, 0.0f, 0.0f, rad(-90.0f));

  double last_frame_time = glfwGetTime();
  while (!glfwWindowShouldClose(display.window)) {
    // handle window events
    glfwPollEvents();

    // calculate delta time
    double current_frame_time = glfwGetTime();
    delta_time = (float)current_frame_time - (float)last_frame_time;
    last_frame_time = current_frame_time;

    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(display.shader_program);

    /* debug cam movement */
    if (keys_down[GLFW_KEY_Q])
      camera->yaw += 1.0f;
    if (keys_down[GLFW_KEY_E])
      camera->yaw -= 1.0f;
    vec3 speed, side;
    if (keys_down[GLFW_KEY_W]) {
      vec3_scale(speed, camera->front, 0.2f);
      vec3_add(camera->position, camera->position, speed);
    }
    if (keys_down[GLFW_KEY_S]) {
      vec3_scale(speed, camera->front, 0.2f);
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

    iso_camera_update(camera, display.shader_program);
    mesh_draw(m, display.shader_program);

    glfwSwapBuffers(display.window);
  }

  printf("Cleaning up\n");
}

void game_exit()
{
  printf("Exiting\n");
  conf_free();
  window_destroy();
  free(camera);
}