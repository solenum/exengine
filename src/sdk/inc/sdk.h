#ifndef SDK_H
#define SDK_K

#include <exengine/exe_conf.h>
#include <exengine/window.h>
#include <exengine/mesh.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdbool.h>

void sdk_init();

void sdk_update(double dt);

void sdk_draw();

void sdk_exit();

#endif // SDK_H