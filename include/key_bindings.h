#ifndef _KEY_BINDINGS_H
#define _KEY_BINDINGS_H

#include "graphics.h"

#define KEY_PRESS         GLFW_PRESS
#define KEY_RELEASE       GLFW_RELEASE

#define KEY_PRESSED(_window, _key) glfwGetKey(_window, _key) == KEY_PRESS
#define KEY_RELEASED(_window, _key) glfwGetKey(_window, _key) == KEY_RELEASE

#define MOUSE_BUTTON_LEFT     GLFW_MOUSE_BUTTON_LEFT
#define MOUSE_BUTTON_RIGHT    GLFW_MOUSE_BUTTON_RIGHT

#define K_ENTER           GLFW_KEY_ENTER
#define K_ESCAPE          GLFW_KEY_ESCAPE
#define K_SPACE           GLFW_KEY_SPACE
#define K_BACKSPACE       GLFW_KEY_BACKSPACE
#define K_TAB             GLFW_KEY_TAB
#define K_CAPS_LOCK       GLFW_KEY_CAPS_LOCK
#define K_LSHIFT          GLFW_KEY_LEFT_SHIFT
#define K_RSHIFT          GLFW_KEY_RIGHT_SHIFT
#define K_LCTRL           GLFW_KEY_LEFT_CONTROL
#define K_RCTRL           GLFW_KEY_RIGHT_CONTROL
#define K_LALT            GLFW_KEY_LEFT_ALT
#define K_RALT            GLFW_KEY_RIGHT_ALT

#define K_RIGHT           GLFW_KEY_RIGHT
#define K_LEFT            GLFW_KEY_LEFT
#define K_DOWN            GLFW_KEY_DOWN
#define K_UP              GLFW_KEY_UP

#define K_Q               GLFW_KEY_Q
#define K_W               GLFW_KEY_W
#define K_E               GLFW_KEY_E
#define K_R               GLFW_KEY_R
#define K_T               GLFW_KEY_T
#define K_Y               GLFW_KEY_Y
#define K_U               GLFW_KEY_U
#define K_I               GLFW_KEY_I
#define K_O               GLFW_KEY_O
#define K_P               GLFW_KEY_P
#define K_A               GLFW_KEY_A
#define K_S               GLFW_KEY_S
#define K_D               GLFW_KEY_D
#define K_F               GLFW_KEY_F
#define K_G               GLFW_KEY_G
#define K_H               GLFW_KEY_H
#define K_J               GLFW_KEY_J
#define K_K               GLFW_KEY_K
#define K_L               GLFW_KEY_L
#define K_Z               GLFW_KEY_Z
#define K_X               GLFW_KEY_X
#define K_C               GLFW_KEY_C
#define K_V               GLFW_KEY_V
#define K_B               GLFW_KEY_B
#define K_N               GLFW_KEY_N
#define K_M               GLFW_KEY_M

#define K_0               GLFW_KEY_0
#define K_1               GLFW_KEY_1
#define K_2               GLFW_KEY_2
#define K_3               GLFW_KEY_3
#define K_4               GLFW_KEY_4
#define K_5               GLFW_KEY_5
#define K_6               GLFW_KEY_6
#define K_7               GLFW_KEY_7
#define K_8               GLFW_KEY_8
#define K_9               GLFW_KEY_9

#endif // _KEY_BINDINGS_H
