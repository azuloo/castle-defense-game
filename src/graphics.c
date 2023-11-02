#include "graphics.h"
#include "utils.h"
#include "file_reader.h"
#include "stb_image.h"

#include <stdlib.h>

// TODO: Review capacity
typedef struct {
	unsigned int  vbo;
	unsigned int  vao;
	unsigned int  ebo;
	unsigned int  shader_prog;
	unsigned int  texture;
	float         data[32];
} VertexUnit32;

#define PROGRAM_IV_LOG_BUF_CAPACITY 512

static GLFWwindow* _window = NULL;
static int g_VertexDataCapacity32 = 1;
static int g_UnitsNum32 = 0;
static VertexUnit32* VertexData32 = NULL;
static BackgroundColor g_BColor = { 0.2f, 0.3f, 0.3f, 1.0f };
static char* g_VertexShaderFilePath = "/res/vertex_source.TXT";
static char* g_FragShaderFilePath = "/res/fragment_source.TXT";

static void _graphics_terminate(int code)
{
	graphics_free_resources();
	exit(code);
}

static void _check_program_iv(unsigned int prog, unsigned int opcode, const char* msg)
{
	int success;
	char info_log[PROGRAM_IV_LOG_BUF_CAPACITY];
	glGetProgramiv(prog, opcode, &success);
	if (!success)
	{
		glGetProgramInfoLog(prog, 512, NULL, info_log);
		PRINT_ERR_VARGS(msg, info_log);
	}
}

static char* _get_file_path(const char* name)
{
	return str_concat(STRVAL(SOURCE_ROOT), name);
}

static char* _get_shader_source(const char* name)
{
	char* vertex_source_path = _get_file_path(name);
	char* data_buf = '\0';
	size_t shader_size = 0;

	int res_code = readall(vertex_source_path, &data_buf, &shader_size);
	if (READ_OK != res_code)
	{
		PRINT_ERR_VARGS("Failed to load vertex source '%s', err code: %d.", name, res_code);
		_graphics_terminate(TERMINATE_ERR_CODE);
	}

	return data_buf;
}

static void _compile_shader_program(unsigned int* prog, unsigned int vertexShader, unsigned int fragShader)
{
	*prog = glCreateProgram();
	glAttachShader(*prog, vertexShader);
	glAttachShader(*prog, fragShader);
	glLinkProgram(*prog);

	_check_program_iv(*prog, GL_LINK_STATUS, "Failed to link shader program: %s\n");

	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
}

unsigned int _create_vertex_shader(const char** source)
{
	unsigned int shader;

	shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader, 1, source, NULL);
	glCompileShader(shader);

	_check_program_iv(shader, GL_COMPILE_STATUS, "Failed to compile vertex shader: %s\n");

	return shader;
}

unsigned int _create_fragment_shader(const char** source)
{
	unsigned int shader;

	shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader, 1, source, NULL);
	glCompileShader(shader);

	_check_program_iv(shader, GL_COMPILE_STATUS, "Failed to compile fragment shader: %s\n");

	return shader;
}

static void _framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void _process_input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, CLOSE_GLFW_WINDOW);
	}
}

static void _init_glfw()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, LIB_MAJOR_VER);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, LIB_MINOR_VER);
	glfwWindowHint(GLFW_OPENGL_PROFILE, LIB_DEFUALT_PROFILE);
}

static void _set_context_current(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);
}

static GLFWwindow* _create_window()
{
	GLFWwindow* window = glfwCreateWindow(WINDOW_DEFAULT_RES_W, WINDOW_DEFAULT_RES_H, WINDOW_DEFUALT_NAME, NULL, NULL);
	if (NULL == window)
	{
		PRINT_ERR("Failed to init window.");
		glfwTerminate();
	}

	_set_context_current(window);
	glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);

	return window;
}

static void _init_glad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		PRINT_ERR("Failed to init GLAD.");
		glfwTerminate();
	}
}

static void _set_viewport(int width, int height)
{
	glViewport(0, 0, width, height);
}

static void _create_vbo(unsigned int* vbo, float* vertices, int len)
{
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * len, vertices, GL_STATIC_DRAW);
}

static void _create_vao(unsigned int* vao)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
}

static void _create_ebo(unsigned int* ebo, unsigned int* indices, int len)
{
	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * len, indices, GL_STATIC_DRAW);
}

static void _init_vertex_array32()
{
	// TODO: resolve capacity thing
	g_VertexDataCapacity32 *= 2;
	VertexUnit32* unit_arr = (VertexUnit32*) realloc(VertexData32, sizeof(VertexUnit32) * g_VertexDataCapacity32);
	if (NULL == unit_arr)
	{
		PRINT_ERR("Failed to allocate sufficient memory chunk for VertexUnit32 elements.");
		_graphics_terminate(TERMINATE_ERR_CODE);
		return;
	}

	VertexData32 = unit_arr;
}

VertexUnit32* _create_vertex_array32_entry()
{
	if (g_UnitsNum32 >= g_VertexDataCapacity32)
	{
		_init_vertex_array32();
	}

	VertexUnit32* unit  = VertexData32 + g_UnitsNum32;
	unit->vbo           = 0;
	unit->vao           = 0;
	unit->ebo           = 0;
	unit->shader_prog   = 0;
	unit->texture       = 0;
	for (int i = 0.f; i < 32; i++)
	{
		unit->data[i]   = 0.f;
	}

	g_UnitsNum32++;

	return unit;
}

VertexUnit32* _create_entry32(float* vertices, int len)
{
	if (NULL == VertexData32)
	{
		_init_vertex_array32();
	}
	
	// TODO: assert len <= 32

	VertexUnit32* entry = _create_vertex_array32_entry();
	// TODO: Do we need to save vertex data?
	float* vertex_data = entry->data;
	for (int i = 0; i < len; i++)
	{
		*(vertex_data++) = vertices[i];
	}

	return entry;
}

void _free_gl_resources()
{
	for (int i = 0; i < g_UnitsNum32; i++)
	{
		VertexUnit32* unit = VertexData32 + i;
		glDeleteVertexArrays(1, &unit->vao);
		glDeleteBuffers(1, &unit->vbo);
		glDeleteProgram(unit->shader_prog);
	}
}

void _free_entries32()
{
	free(VertexData32);
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int graphics_should_be_terminated()
{
	return glfwWindowShouldClose(_window);
}

void graphics_free_resources()
{
	_free_gl_resources();
	_free_entries32();

	glfwTerminate();
}

static unsigned char* _load_image(const char* path, int* width, int* height, int* nr_channels)
{
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load(path, width, height, nr_channels, 0);
	return data;
}

static void _free_img_data(unsigned int* img_data)
{
	stbi_image_free(img_data);
}

// TODO: Return and store textures
void _create_texture_2D(const char* img_path, unsigned int* texture)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	// TODO: Configure filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nr_channels;
	unsigned char* data = _load_image(img_path, &width, &height, &nr_channels);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		PRINT_ERR("Failed to load texture.");
	}
	_free_img_data(data);
}

int draw_triangle(float vertices[], int vertices_len, unsigned int indices[], int indices_len)
{
	// TODO: Do we need to create a new 32 entry for each supplied number of vertices?
	// Probablty not and we sholud probably bind minimum a couple of vertices batches to the same vao, vbo and shader_prog
	VertexUnit32* entry = _create_entry32(vertices, vertices_len);

	const char* vertex_shader_source = _get_shader_source(g_VertexShaderFilePath);
	const char* fragment_shader_source = _get_shader_source(g_FragShaderFilePath);

	unsigned int vertex_shader = _create_vertex_shader(&vertex_shader_source);
	unsigned int fragment_shader = _create_fragment_shader(&fragment_shader_source);

	_compile_shader_program(&entry->shader_prog, vertex_shader, fragment_shader);

	_create_vao(&entry->vao);
	_create_vbo(&entry->vbo, entry->data, vertices_len);
	_create_ebo(&entry->ebo, indices, indices_len);
	// TODO: Should be called by user
	const char* texure_name = "/res/brick.jpg";
	_create_texture_2D(_get_file_path(texure_name), &entry->texture);

	// TODO: Move these out of here?
	// TODO: handle different attributes
	glVertexAttribPointer(0,	// vertex attribute index
		3,						// size of vertex attribute (vec3)
		GL_FLOAT,				// data type
		GL_FALSE,				// normalize?
		sizeof(float) * 8,		// stride
		(void*)0				// position data offset
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1,	     // vertex attribute index
		3,						     // size of vertex attribute (vec3)
		GL_FLOAT,				     // data type
		GL_FALSE,				     // normalize?
		sizeof(float) * 8,		     // stride
		(void*)(sizeof(float) * 3)   // position data offset
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2,	     // vertex attribute index
		2,						     // size of vertex attribute (vec3)
		GL_FLOAT,				     // data type
		GL_FALSE,				     // normalize?
		sizeof(float) * 8,		     // stride
		(void*)(sizeof(float) * 6)   // position data offset
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	return 0;
}

int init_graphics()
{
	// TODO: init_graphics() called before everything assertion (gl and GLAD)
	_init_glfw();
	_window = _create_window();
	_init_glad();

	return 0;
}

void set_background_color(BackgroundColor b_color)
{
	g_BColor = b_color;
}

int draw()
{
	_process_input(_window);

	glClearColor(g_BColor.R, g_BColor.G, g_BColor.B, g_BColor.A);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < g_UnitsNum32; i++)
	{
		VertexUnit32* entry = VertexData32 + i;
		glUseProgram(entry->shader_prog);
		glBindTexture(GL_TEXTURE_2D, entry->texture);
		glBindVertexArray(entry->vao);

		//glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entry->ebo);
		// TODO: Get the number of vertices from the entry
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	glfwSwapBuffers(_window);
	glfwPollEvents();
	
	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
