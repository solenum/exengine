#ifndef EX_IMGUI_H
#define EX_IMGUI_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>

#include "window.h"

typedef struct ImDrawVert ImDrawVert;

extern bool glimgui_focus;
extern uint8_t glimgui_ex_keys_down[GLFW_KEY_LAST];
extern bool igmousepressed[3];

void glimgui_init(GLFWwindow *win);

void glimgui_renderdrawlists(struct ImDrawData *draw_data);

void glimgui_createfontstexture();

void glimgui_createobjects();

void glimgui_invalidateobjects();

void glimgui_shutdown();

void glimgui_newframe();

static const char* glimgui_getclipboard(void *user_data);

static void glimgui_setclipboard(void *user_data, const char *text);

void glimgui_mousebuttoninput(int button, int action);

void glimgui_mousescrollinput(double offset);

void glimgui_keyinput(int key, int action);

void glimgui_charinput(unsigned int c);

#endif // EX_IMGUI_H