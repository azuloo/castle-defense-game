#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "lin_alg.h"
#include "global_decl.h"

#define CLOSE_GLFW_WINDOW 1
#define LIB_MAJOR_VER 3
#define LIB_MINOR_VER 3
#define LIB_DEFUALT_PROFILE GLFW_OPENGL_CORE_PROFILE

#define DRAW_MODE_STATIC     GL_STATIC_DRAW
#define DRAW_MODE_DYNAMIC    GL_DYNAMIC_DRAW
#define DRAW_MODE_STREAM     GL_STREAM_DRAW

int check_graphics_initialized();
#define ASSERT_GRAPHICS_INITIALIZED assert( check_graphics_initialized() == 1 );

enum TextureType
{
	TexType_RGB,
	TexType_RGBA,
	TexType_RED
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
	unsigned int vbo;
	unsigned int vao;
	unsigned int ebo;
} GBuffers;

typedef struct TransformDef
{
	Vec3 translation;
	Vec3 rotation;
	Vec3 scale;
} TransformDef;

typedef struct DrawBufferData
{
	float*          vertices;
	int             vertices_len;
	int*            indices;
	int             indices_len;
} DrawBufferData;

typedef struct DrawableDef
{
	GBuffers          buffers;
	DrawBufferData    buffer_data;
	unsigned int      shader_prog;
	unsigned int      texture;
	unsigned int      num_indices;
	GAttributes*      attributes;
	GMatrices         matrices;
	TransformDef      transform;
	TransformDef      init_transform;
	int               handle;
	int               visible;
	int               draw_mode;
} DrawableDef;

typedef struct GLFWwindow GWindow;
typedef void (*InputFnPtr)(GWindow* window);
typedef void (*WindowResizeFnPtr)(GWindow* window, int width, int height);
typedef void (*KeyCbPtr)(GWindow* window, int key, int scancode, int action, int mods);
typedef void (*MouseButtonCbPtr)(GWindow* window, int button, int action, int mods);

// Graphics sub-module functions

int create_vbo(unsigned int* vbo, float* vertices, int len, int draw_mode);
int create_vao(unsigned int* vao);
int create_ebo(unsigned int* ebo, unsigned int* indices, int len, int draw_mode);
int compile_shaders(unsigned int* shader_prog, const char* vertex_shader_src, const char* fragment_shader_src);
// ! Allocates memory on heap (indirect) !
char* get_shader_source(const char* name);
// ! Allocates memory on heap !
int create_drawable_buffer_data(DrawableDef* drawable, DrawBufferData* src);

// Graphics sub-module functions end

void close_window(GWindow* window);
void bind_input_fn(InputFnPtr ptr);
void bind_window_resize_fn(WindowResizeFnPtr ptr);
void bind_key_pressed_cb(KeyCbPtr ptr);
void bind_mouse_button_cb(MouseButtonCbPtr ptr);
int graphics_get_cursor_pos(double* xpos, double* ypos);

int init_graphics();
void set_background_color(BackgroundColor b_color);
void drawable_set_visible(DrawableDef* drawable, int visible);
int get_drawable_def(DrawableDef** dest, int handle);
int graphics_should_be_terminated();
void graphics_free_resources();
void set_unpack_alignment(int align);

int create_drawable(DrawableDef** dest);
int create_texture_2D(unsigned char* data, int width, int height, unsigned int* texture, enum TextureType type);
int add_uniform_mat4f(unsigned int shader_prog, const char* uniform_name, const Mat4* mat);
int add_uniform_vec4f(unsigned int shader_prog, const char* uniform_name, const Vec4* vec);
int add_uniform_vec3f(unsigned int shader_prog, const char* uniform_name, const Vec3* vec);
int add_texture_2D(DrawableDef* drawable, const char* texture_path, int texture_type);
int setup_drawable(DrawableDef* drawable, const DrawBufferData* buf_data, const char* vertex_shader_path, const char* fragment_shader_path);
int register_drawable_attribute(DrawableDef* drawable, unsigned int size);
int process_drawable_attributes(DrawableDef* drawable);
int graphics_draw();

#endif // _GRAPHICS_H
