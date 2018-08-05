#ifndef EX_INPUT_H
#define EX_INPUT_H

#include <inttypes.h>
#include "glimgui.h"

/* defines copied over from glfw3.h */
/* The unknown key */
#define EX_KEY_UNKNOWN            -1

/* Printable keys */
#define EX_KEY_SPACE              32
#define EX_KEY_APOSTROPHE         39  /* ' */
#define EX_KEY_COMMA              44  /* , */
#define EX_KEY_MINUS              45  /* - */
#define EX_KEY_PERIOD             46  /* . */
#define EX_KEY_SLASH              47  /* / */
#define EX_KEY_0                  48
#define EX_KEY_1                  49
#define EX_KEY_2                  50
#define EX_KEY_3                  51
#define EX_KEY_4                  52
#define EX_KEY_5                  53
#define EX_KEY_6                  54
#define EX_KEY_7                  55
#define EX_KEY_8                  56
#define EX_KEY_9                  57
#define EX_KEY_SEMICOLON          59  /* ; */
#define EX_KEY_EQUAL              61  /* = */
#define EX_KEY_A                  65
#define EX_KEY_B                  66
#define EX_KEY_C                  67
#define EX_KEY_D                  68
#define EX_KEY_E                  69
#define EX_KEY_F                  70
#define EX_KEY_G                  71
#define EX_KEY_H                  72
#define EX_KEY_I                  73
#define EX_KEY_J                  74
#define EX_KEY_K                  75
#define EX_KEY_L                  76
#define EX_KEY_M                  77
#define EX_KEY_N                  78
#define EX_KEY_O                  79
#define EX_KEY_P                  80
#define EX_KEY_Q                  81
#define EX_KEY_R                  82
#define EX_KEY_S                  83
#define EX_KEY_T                  84
#define EX_KEY_U                  85
#define EX_KEY_V                  86
#define EX_KEY_W                  87
#define EX_KEY_X                  88
#define EX_KEY_Y                  89
#define EX_KEY_Z                  90
#define EX_KEY_LEFT_BRACKET       91  /* [ */
#define EX_KEY_BACKSLASH          92  /* \ */
#define EX_KEY_RIGHT_BRACKET      93  /* ] */
#define EX_KEY_GRAVE_ACCENT       96  /* ` */
#define EX_KEY_WORLD_1            161 /* non-US #1 */
#define EX_KEY_WORLD_2            162 /* non-US #2 */
/* Function keys */
#define EX_KEY_ESCAPE             256
#define EX_KEY_ENTER              257
#define EX_KEY_TAB                258
#define EX_KEY_BACKSPACE          259
#define EX_KEY_INSERT             260
#define EX_KEY_DELETE             261
#define EX_KEY_RIGHT              262
#define EX_KEY_LEFT               263
#define EX_KEY_DOWN               264
#define EX_KEY_UP                 265
#define EX_KEY_PAGE_UP            266
#define EX_KEY_PAGE_DOWN          267
#define EX_KEY_HOME               268
#define EX_KEY_END                269
#define EX_KEY_CAPS_LOCK          280
#define EX_KEY_SCROLL_LOCK        281
#define EX_KEY_NUM_LOCK           282
#define EX_KEY_PRINT_SCREEN       283
#define EX_KEY_PAUSE              284
#define EX_KEY_F1                 290
#define EX_KEY_F2                 291
#define EX_KEY_F3                 292
#define EX_KEY_F4                 293
#define EX_KEY_F5                 294
#define EX_KEY_F6                 295
#define EX_KEY_F7                 296
#define EX_KEY_F8                 297
#define EX_KEY_F9                 298
#define EX_KEY_F10                299
#define EX_KEY_F11                300
#define EX_KEY_F12                301
#define EX_KEY_F13                302
#define EX_KEY_F14                303
#define EX_KEY_F15                304
#define EX_KEY_F16                305
#define EX_KEY_F17                306
#define EX_KEY_F18                307
#define EX_KEY_F19                308
#define EX_KEY_F20                309
#define EX_KEY_F21                310
#define EX_KEY_F22                311
#define EX_KEY_F23                312
#define EX_KEY_F24                313
#define EX_KEY_F25                314
#define EX_KEY_KP_0               320
#define EX_KEY_KP_1               321
#define EX_KEY_KP_2               322
#define EX_KEY_KP_3               323
#define EX_KEY_KP_4               324
#define EX_KEY_KP_5               325
#define EX_KEY_KP_6               326
#define EX_KEY_KP_7               327
#define EX_KEY_KP_8               328
#define EX_KEY_KP_9               329
#define EX_KEY_KP_DECIMAL         330
#define EX_KEY_KP_DIVIDE          331
#define EX_KEY_KP_MULTIPLY        332
#define EX_KEY_KP_SUBTRACT        333
#define EX_KEY_KP_ADD             334
#define EX_KEY_KP_ENTER           335
#define EX_KEY_KP_EQUAL           336
#define EX_KEY_LEFT_SHIFT         340
#define EX_KEY_LEFT_CONTROL       341
#define EX_KEY_LEFT_ALT           342
#define EX_KEY_LEFT_SUPER         343
#define EX_KEY_RIGHT_SHIFT        344
#define EX_KEY_RIGHT_CONTROL      345
#define EX_KEY_RIGHT_ALT          346
#define EX_KEY_RIGHT_SUPER        347
#define EX_KEY_MENU               348
#define EX_KEY_LAST               EX_KEY_MENU

#define EX_MOUSE_BUTTON_1         0
#define EX_MOUSE_BUTTON_2         1
#define EX_MOUSE_BUTTON_3         2
#define EX_MOUSE_BUTTON_4         3
#define EX_MOUSE_BUTTON_5         4
#define EX_MOUSE_BUTTON_6         5
#define EX_MOUSE_BUTTON_7         6
#define EX_MOUSE_BUTTON_8         7
#define EX_MOUSE_BUTTON_LAST      EX_MOUSE_BUTTON_8
#define EX_MOUSE_BUTTON_LEFT      EX_MOUSE_BUTTON_1
#define EX_MOUSE_BUTTON_RIGHT     EX_MOUSE_BUTTON_2
#define EX_MOUSE_BUTTON_MIDDLE    EX_MOUSE_BUTTON_3

#define EX_RELEASE                0
#define EX_PRESS                  1
#define EX_REPEAT                 2

#define EX_MOD_SHIFT              0x0001
#define EX_MOD_CONTROL            0x0002
#define EX_MOD_ALT                0x0004
#define EX_MOD_SUPER              0x0008

extern uint8_t ex_keys_down[EX_KEY_LAST];
extern uint8_t ex_buttons_down[EX_MOUSE_BUTTON_LAST];

void ex_key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void ex_button_callback(GLFWwindow *window, int button, int action, int mods);

void ex_char_callback(GLFWwindow *window, unsigned int c);

void ex_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void ex_mouse_callback(GLFWwindow* window, double x, double y);

void ex_get_mouse_pos(double *x, double *y);

#endif // EX_INPUT_H