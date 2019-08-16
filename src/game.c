#include "exengine/camera.h"
#include "exengine/texture.h"
#include "exengine/pointlight.h"
#include "exengine/dirlight.h"
#include "exengine/scene.h"
#include "exengine/exe_list.h"
#include "exengine/iqm.h"
#include "exengine/skybox.h"
#include "exengine/entity.h"
#include "exengine/reflectionprobe.h"
#include "exengine/engine.h"
#include "exengine/input.h"
#include "exengine/sound.h"
#include "exengine/gbuffer.h"
#include "exengine/text.h"
#include "inc/game.h"

ex_fps_camera_t *camera;
ex_scene_t *scene;
ex_model_t *m6, *d, *box;
ex_entity_t *cube, *e;
ex_point_light_t *l, *pl;
ex_source_t *sound;
ex_font_t *font;
float move_speed = 1.5f;

void game_init()
{
  // init the scene
  scene = ex_scene_new(0);
  memcpy(scene->gravity, (vec3){0.0f, -0.1f, 0.0f}, sizeof(vec3));

  // init the camera
  camera = ex_fps_camera_new(0.0f, 0.0f, 0.0f, 0.1f, 70.0f);

  m6 = ex_iqm_load_model(scene, "data/level.iqm", EX_KEEP_VERTICES);
  m6->is_shadow = 1;
  ex_scene_add_model(scene, m6);
 
  e = ex_entity_new(scene, (vec3){0.5f, 1.0f, 0.5f});
  e->position[1] = 10.0f;
  e->position[0] = 5.0f;
  e->position[2] = 0.0f;

  // load a sound
  sound = ex_sound_load_source("data/sound.ogg", EX_SOUND_OGG, 0);
  ex_sound_master_volume(0.5f);

  pl = ex_point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.1f, 0.1f, 0.1f}, 0);
  memcpy(pl->position, e->position, sizeof(vec3));
  ex_scene_add_pointlight(scene, pl);
  pl->is_shadow = 0;

  box = ex_iqm_load_model(scene, "data/cube.iqm", 0);

  box->is_lit = 0;
  box->is_shadow = 0;
  ex_scene_add_model(scene, box);
  cube = ex_entity_new(scene, (vec3){0.95f, 0.95f, 0.95f});
  cube->position[2] = 5.0f;
  cube->position[1] = 5.0f;
  cube->position[0] = 0.0f;

  // this aint it
  font = ex_font_load("data/fonts/OpenSans-Regular.ttf", "abcdefghijklmnopqrstuvwxyzHW!_");
}

void game_update(double dt)
{
  ex_entity_update(e, dt);
  ex_entity_update(cube, dt);

  memcpy(camera->position, e->position, sizeof(vec3));
  camera->position[1] += e->radius[1];
  memcpy(pl->position, camera->position, sizeof(vec3));

  memcpy(box->position, cube->position, sizeof(vec3));

  vec3 temp;
  vec3_sub(temp, cube->position, e->position);
  float len = vec3_len(temp);
  if (len <= cube->radius[1]/2 + e->radius[1]) {
    vec3_norm(temp, temp);
    vec3_scale(temp, temp, vec3_len(e->velocity));
    vec3_add(cube->velocity, cube->velocity, temp);
    vec3_sub(e->velocity, e->velocity, temp);
  }

  vec3_scale(temp, cube->velocity, 5.0f * dt);
  temp[1] = 0.0f;
  if (cube->grounded == 1)
    vec3_sub(cube->velocity, cube->velocity, temp);

  cube->velocity[1] -= (100.0f * dt);

  if (ex_keys_down[EX_KEY_LEFT_CONTROL]) {
    vec3 p;
    vec3_scale(p, camera->front, 2.5f);
    vec3_add(p, p, e->position);
    p[1] += 0.5f;
    
    vec3_sub(p, p, cube->position);

    float f = vec3_len(p);
    if (f > 1.5f) {
      ex_keys_down[EX_KEY_LEFT_CONTROL] = 0;
      goto ctrl_end;
    }

    if (f > 1.0f)
      f *= f;

    vec3_norm(p, p);
    vec3_scale(p, p, f*35.0f);
    f = cube->velocity[1];
    memcpy(cube->velocity, p, sizeof(vec3));
    cube->velocity[1] += f*0.1f;
  
    if (ex_buttons_down[GLFW_MOUSE_BUTTON_RIGHT]) {
      vec3_scale(temp, camera->front, 80.0f);
      vec3_add(cube->velocity, cube->velocity, temp);
      ex_keys_down[EX_KEY_LEFT_CONTROL] = 0;
    }
  }
ctrl_end:

  if (ex_keys_down[EX_KEY_1])
    ex_model_set_anim(d, "Walk");
  if (ex_keys_down[EX_KEY_2])
    ex_model_set_anim(d, "Run");

  if (ex_keys_down[EX_KEY_F]) {
    float r = (float)rand()/(float)(RAND_MAX/1.0f); 
    float g = (float)rand()/(float)(RAND_MAX/1.0f);
    float b = (float)rand()/(float)(RAND_MAX/1.0f);
    ex_point_light_t *l = ex_point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){r, g, b}, 0);
    memcpy(l->position, camera->position, sizeof(vec3));
    ex_scene_add_pointlight(scene, l);
    l->is_shadow = 1;
    ex_keys_down[EX_KEY_F] = 0;
  }

  /* debug entity movement */
  vec3_scale(temp, e->velocity, 15.0f * dt);
  temp[1] = 0.0f;

  if (e->grounded == 1) 
   vec3_sub(e->velocity, e->velocity, temp);
  else
    move_speed = 50.0f;
  
  e->velocity[1] -= (100.0f * dt);

  if (ex_keys_down[EX_KEY_C])
    glfwSwapInterval(1);
  if (ex_keys_down[EX_KEY_V])
    glfwSwapInterval(0);

  vec3 speed, side;
  if (ex_keys_down[EX_KEY_W]) {
    vec3_norm(speed, (vec3){camera->front[0], 0.0f, camera->front[2]});
    vec3_scale(speed, speed, move_speed * dt);
    speed[1] = 0.0f;
    vec3_add(e->velocity, e->velocity, speed);
  }
  if (ex_keys_down[EX_KEY_S]) {
    vec3_norm(speed, (vec3){camera->front[0], 0.0f, camera->front[2]});
    vec3_scale(speed, speed, move_speed * dt); 
    speed[1] = 0.0f;
    vec3_sub(e->velocity, e->velocity, speed);
  }
  if (ex_keys_down[EX_KEY_A]) {
    vec3_mul_cross(side, camera->front, camera->up);
    vec3_norm(side, side);
    vec3_scale(side, side, (move_speed*0.9f) * dt);
    side[1] = 0.0f;
    vec3_sub(e->velocity, e->velocity, side);
  }
  if (ex_keys_down[EX_KEY_D]) {
    vec3_mul_cross(side, camera->front, camera->up);
    vec3_norm(side, side);
    vec3_scale(side, side, (move_speed*0.9f) * dt);
    side[1] = 0.0f;
    vec3_add(e->velocity, e->velocity, side);
  }
  if (ex_keys_down[EX_KEY_Q])
    e->velocity[1] = 50.0f;
  if (ex_keys_down[EX_KEY_Z])
    e->velocity[1] = -50.0f;
  if (ex_keys_down[EX_KEY_SPACE] && e->grounded == 1) {
    e->velocity[1] = 20.0f;
    if (e->grounded && !ex_sound_playing(sound)) {
      alSourcePlay(sound->id);
    }
  }
  move_speed = 100.0f;

  memcpy(pl->position, e->position, sizeof(vec3));
  pl->position[1] += 1.0f;
  ex_scene_update(scene, dt);
  ex_fps_camera_update(camera);
}

void game_draw()
{
  ex_scene_draw(scene, 0, 0, 0, 0, &camera->matrices);
  ex_fps_camera_resize(camera);

  ex_font_dbg(font);
}

void game_exit()
{
  ex_scene_destroy(scene);
  printf("Exiting\n");
}

void game_keypressed(int key, int scancode, int action, int mode)
{

}

void game_resize(int width, int height)
{
  ex_scene_resize(scene, width, height);
}
