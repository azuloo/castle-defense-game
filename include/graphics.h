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

static char err_msg_buffer[1024];

#define PRINT_ERR(msg) fprintf(stderr, "ERR: %s:%d: %s\n", __FILE__, __LINE__, msg)
// TODO: Is there a better way to do this?
#define PRINT_ERR_VARGS(msg, ...) \
	sprintf(err_msg_buffer, msg, __VA_ARGS__); \
	PRINT_ERR(err_msg_buffer); \
	memset(err_msg_buffer, 0, sizeof(err_msg_buffer));

typedef struct {
	float R;
	float G;
	float B;
	float A;
} BackgroundColor;

typedef struct {
	unsigned int  vbo;
	unsigned int  vao;
	unsigned int  ebo;
	unsigned int  shader_prog;
	unsigned int  texture;
} EntryCnf;

typedef struct {
	float*       vertices;
	unsigned int vertices_len;
	float*       indices;
	unsigned int indices_len;
} DrawBufferData;

extern int init_graphics();
extern void set_background_color(BackgroundColor b_color);
extern int graphics_should_be_terminated();
extern void graphics_free_resources();

extern EntryCnf* create_entry();
extern int draw_triangle(EntryCnf* entry, DrawBufferData* buf_data);
extern int draw();

#endif // _GRAPHICS_H
