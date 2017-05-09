#include <scene.h>
#include <mesh.h>
#include <pointlight.h>
#include <framebuffer.h>
#include <stdlib.h>


scene_t* scene_new()
{
  // load shaders
  scene_t *s = malloc(sizeof(scene_t));
  s->shader = shader_compile("data/shader.vs", "data/shader.fs", NULL);

  // init lists
  s->mesh_list        = list_new();
  s->point_light_list = list_new();

  s->fps_camera = NULL;

  // init framebuffers etc
  framebuffer_init();

  // init point lights
  point_light_init();

  return s;
}

void scene_update(scene_t *s, float delta_time)
{
  list_node_t *n = s->mesh_list;
  while (n->data != NULL) {
    mesh_update((mesh_t*)n->data, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

void scene_draw(scene_t *s)
{
  // render pointlight depth maps
  list_node_t *n = s->point_light_list;
  while (n->data != NULL) {
    point_light_t *l = n->data;
    point_light_begin(l);
    glCullFace(GL_FRONT);
    scene_render_meshes(s, l->shader);
    glCullFace(GL_BACK);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // first rendering pass
  framebuffer_first();

  // render everything
  glUseProgram(s->shader);

  // update camera
  if (s->fps_camera != NULL)
    fps_camera_update(s->fps_camera, s->shader);

  // render ambient pass
  glUniform1i(glGetUniformLocation(s->shader, "u_ambient_pass"), 1);
  glDisable(GL_BLEND);
  scene_render_meshes(s, s->shader);

  // render point lights
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glUniform1i(glGetUniformLocation(s->shader, "u_ambient_pass"), 0);
  n = s->point_light_list;
  while (n->data != NULL) {
    point_light_t *l = n->data;
    point_light_draw(l, s->shader);

    // render meshes
    scene_render_meshes(s, s->shader);
    
    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
  glDisable(GL_BLEND);


  // render screen quad
  framebuffer_render_quad();
}

void scene_render_meshes(scene_t *s, GLuint shader)
{
  list_node_t *n = s->mesh_list;
  while (n->data != NULL) {
    mesh_draw((mesh_t*)n->data, shader);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

void scene_destroy(scene_t *s)
{
  printf("Cleaning up scene\n");

  // cleanup meshes
  list_node_t *n = s->mesh_list;
  while (n->data != NULL) {
    mesh_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free mesh list
  list_destroy(s->mesh_list);

  // cleanup point lights
  n = s->point_light_list;
  while (n->data != NULL) {
    point_light_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  list_destroy(s->point_light_list);

  // cleanup cameras
  if (s->fps_camera != NULL)
    free(s->fps_camera);

  // cleanup framebuffers
  framebuffer_destroy();
}