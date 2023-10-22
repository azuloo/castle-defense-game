#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdlib.h>

#define CLOSE_GLFW_WINDOW 1
#define LIB_MAJOR_VER 3
#define LIB_MINOR_VER 3
#define LIB_DEFUALT_PROFILE GLFW_OPENGL_CORE_PROFILE
#define WINDOW_DEFAULT_RES_W 1280
#define WINDOW_DEFAULT_RES_H 800
#define WINDOW_DEFUALT_NAME "Application"
#define GRAPHICS_TERMINATE_SUCCESS_CODE EXIT_SUCCESS
#define GRAPHICS_TERMINATE_ERR_CODE EXIT_FAILURE

#define CHECK_PROG_IV(prog, opcode, msg) { \
								int success; \
								char infoLog[512]; \
								glGetProgramiv(prog, opcode, &success); \
								if (!success) \
								{ \
									glGetProgramInfoLog(prog, 512, NULL, infoLog); \
									printf(msg, infoLog); \
								} \
							}

#define PRINT_ERR(msg) \
    fprintf(stderr, "ERR: %s:%d: %s", __FILE__, __LINE__, msg);

typedef struct {
	float R;
	float G;
	float B;
	float A;
} BackgroundColor;

extern int init_graphics();
extern void set_background_color(BackgroundColor b_color);
extern int graphics_should_be_terminated();
extern void graphics_free_resources();
extern int draw_triangle(float vertices[], int vertices_len, unsigned int indices[], int indices_len);
extern int draw();

#endif // _GRAPHICS_H
