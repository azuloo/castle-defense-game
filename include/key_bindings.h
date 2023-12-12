#ifndef _KEY_BINDINGS_H
#define _KEY_BINDINGS_H

#include "GLFW/glfw3.h"

#define KEY_PRESSED(_window, _key) glfwGetKey(_window, _key) == GLFW_PRESS

#define K_ESCAPE          GLFW_KEY_ESCAPE
#define K_W               GLFW_KEY_W
#define K_S               GLFW_KEY_S
#define K_A               GLFW_KEY_A
#define K_D               GLFW_KEY_D

#endif // _KEY_BINDINGS_H
