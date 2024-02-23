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

#define DRAW_MODE_STATIC     GL_STATIC_DRAW
#define DRAW_MODE_DYNAMIC    GL_DYNAMIC_DRAW
#define DRAW_MODE_STREAM     GL_STREAM_DRAW

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

typedef struct TransformDef
{
	Vec3 translation;
	Vec3 rotation;
	Vec3 scale;
} TransformDef;

typedef struct
{
	unsigned int    vbo;
	unsigned int    vao;
	unsigned int    ebo;
	unsigned int    shader_prog;
	unsigned int    texture;
	unsigned int    num_indices;
	GAttributes*    attributes;
	GMatrices*      matrices;
	TransformDef*   transform;
	int             handle;
	int             visible;
	int             draw_mode;
} DrawableDef;

typedef struct
{
	float*          vertices;
	unsigned int    vertices_len;
	int*            indices;
	unsigned int    indices_len;
} DrawBufferData;

typedef struct GLFWwindow GWindow;
typedef void (*InputFnPtr)(GWindow* window);
typedef void (*WindowResizeFnPtr)(GWindow* window, int width, int height);

void close_window(GWindow* window);
void bind_input_fn(InputFnPtr ptr);
void bind_window_resize_fn(WindowResizeFnPtr ptr);

int init_graphics();
void set_background_color(BackgroundColor b_color);
void drawable_set_visible(DrawableDef* drawable, int visible);
int graphics_should_be_terminated();
void graphics_free_resources();

DrawableDef* create_drawable();
int create_texture_2D(unsigned char* data, int width, int height, unsigned int* texture, enum TextureType type);
int add_uniform_mat4f(unsigned int shader_prog, const char* uniform_name, const Mat4* mat);
int add_uniform_vec4f(unsigned int shader_prog, const char* uniform_name, const Vec4* vec);
int config_drawable(DrawableDef* drawable, const DrawBufferData* buf_data, const char* vertex_shader_path, const char* fragment_shader_path);
int register_drawable_attribute(DrawableDef* drawable, unsigned int size);
int process_drawable_attributes(DrawableDef* drawable);
int graphics_draw();

#endif // _GRAPHICS_H
