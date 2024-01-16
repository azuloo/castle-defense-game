#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "lin_alg.h"
#include "utils.h"

#define CLOSE_GLFW_WINDOW 1
#define LIB_MAJOR_VER 3
#define LIB_MINOR_VER 3
#define LIB_DEFUALT_PROFILE GLFW_OPENGL_CORE_PROFILE
#define WINDOW_DEFAULT_RES_W 1800
#define WINDOW_DEFAULT_RES_H 920
#define WINDOW_DEFUALT_NAME "Application"

enum TextureType
{
	TexType_RGB,
	TexType_RGBA
};

typedef struct
{
	float R;
	float G;
	float B;
	float A;
} BackgroundColor;

typedef struct
{
	unsigned int idx;
	unsigned int size;
} AttributeCnf;

typedef struct
{
	unsigned int     stride;
	unsigned int     type;
	unsigned int     normalize;
	unsigned int     capacity;
	unsigned int     count;
	AttributeCnf*    elements;
} GAttributes; 

typedef struct
{
	Mat4 model;
	Mat4 view;
	Mat4 projection;
} GMatrices;

typedef struct
{
	unsigned int  vbo;
	unsigned int  vao;
	unsigned int  ebo;
	unsigned int  shader_prog;
	unsigned int  texture;
	unsigned int  num_indices;
	GAttributes*  attributes;
	GMatrices*    matrices;
	int           handle;
} EntryCnf;

typedef struct
{
	float*       vertices;
	unsigned int vertices_len;
	float*       indices;
	unsigned int indices_len;
} DrawBufferData;

typedef struct GLFWwindow GWindow;
typedef void (*InputFnPtr)(GWindow* window);
typedef void (*WindowResizeFnPtr)(GWindow* window, float width, float height);

void close_window(GWindow* window);
void bind_input_fn(InputFnPtr ptr);
void bind_window_resize_fn(WindowResizeFnPtr ptr);

int init_graphics();
void set_background_color(BackgroundColor b_color);
int graphics_should_be_terminated();
void graphics_free_resources();

EntryCnf* create_entry();
int create_texture_2D(const char* img_path, unsigned int* texture, enum TextureType type);
int add_uniform_mat4f(unsigned int shader_prog, const char* uniform_name, const Mat4* mat);
int add_element(EntryCnf* entry, DrawBufferData* buf_data, const char* vertex_shader_path, const char* fragment_shader_path);
int add_entry_attribute(EntryCnf* entry, unsigned int size);
int apply_entry_attributes(EntryCnf* entry);
int draw();

#endif // _GRAPHICS_H
