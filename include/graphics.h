#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "lin_alg.h"

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
	unsigned int  num_indices;
} EntryCnf;

typedef struct {
	float*       vertices;
	unsigned int vertices_len;
	float*       indices;
	unsigned int indices_len;
} DrawBufferData;

int init_graphics();
void set_background_color(BackgroundColor b_color);
int graphics_should_be_terminated();
void graphics_free_resources();

EntryCnf* create_entry();
void create_texture_2D(const char* img_path, unsigned int* texture);
void rotate_element_x(unsigned int shader_prog, const char* uniform_name, float angle);
void rotate_element_y(unsigned int shader_prog, const char* uniform_name, float angle);
void rotate_element_z(unsigned int shader_prog, const char* uniform_name, float angle);
void add_uniform_mat4f(unsigned int shader_prog, const char* uniform_name, const Mat4* mat);
int add_element(EntryCnf* entry, DrawBufferData* buf_data);
int draw();

#endif // _GRAPHICS_H
