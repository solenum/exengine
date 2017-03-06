#include <window.h>
#include <stdio.h>

window_t display;
uint8_t keys_down[GLFW_KEY_LAST];

bool window_init(uint32_t width, uint32_t height, const char *title)
{
  // init glfw
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
  // create glfw window
  display.window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (display.window == NULL) {
    printf("Failed to initialize window\n");
    glfwTerminate();
    return false;
  }

  // set callbacks
  glfwSetKeyCallback(display.window, key_callback);
  glfwSetCursorPosCallback(display.window, mouse_callback);
  glfwSetFramebufferSizeCallback(display.window, resize_callback);

  // set context
  glfwMakeContextCurrent(display.window);

  // init glew
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("Error initializing glew\n%s\n", glewGetErrorString(err));
    return false;
  }
  
  // set viewport etc
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glCullFace(GL_BACK);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);

  glfwSetInputMode(display.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  return true;
}

void window_init_shaders(const char *vertex_path, const char *fragment_path)
{
  display.shader_program = shader_compile(vertex_path, fragment_path);
}

void window_destroy()
{
  glfwTerminate();
}

void resize_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
  iso_camera_resize(camera);
}

void mouse_callback(GLFWwindow* window, double x, double y)
{
  float offset_x = x - camera->last_x;
  float offset_y = camera->last_y - y;
  camera->last_x = x;
  camera->last_y = y;

  offset_x *= camera->sensitivity;
  offset_y *= camera->sensitivity;

  camera->yaw += offset_x;
  camera->pitch += offset_y;

  if(camera->pitch > 89.0f)
      camera->pitch = 89.0f;
  if(camera->pitch < -89.0f)
      camera->pitch = -89.0f;
}