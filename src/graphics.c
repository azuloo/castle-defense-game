#include "graphics.h"
#include <stdlib.h>

static GLFWwindow* _window = NULL;

typedef struct {
	unsigned int  vbo;
	unsigned int  vao;
	unsigned int  shader_prog;
	float         data[32];
} VertexUnit32;

static int g_VertexDataCapacity32 = 1;
static int g_UnitsNum32 = 0;
static VertexUnit32* VertexData32 = NULL;
static BackgroundColor g_BColor = { 0.2f, 0.3f, 0.3f, 1.0f };

static const char* k_vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
"}\0";

static const char* k_fragShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0f, 0.5f, 0.5f, 1.0f);\n"
"}\n"
"";

_compile_shader_program(unsigned int* prog, unsigned int vertexShader, unsigned int fragShader)
{
	*prog = glCreateProgram();
	glAttachShader(*prog, vertexShader);
	glAttachShader(*prog, fragShader);
	glLinkProgram(*prog);

	CHECK_PROG_IV(*prog, GL_LINK_STATUS, "ERROR::SHADER::PROGRAM::LINKING_FAILED: %s\n");

	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
}

unsigned int _create_vertex_shader(const char* source)
{
	unsigned int shader;

	shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	CHECK_PROG_IV(shader, GL_COMPILE_STATUS, "ERROR::SHADER::PROGRAM::COMPILATION_FAILED: %s\n");

	return shader;
}

unsigned int _create_fragment_shader(const char* source)
{
	unsigned int shader;

	shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	CHECK_PROG_IV(shader, GL_COMPILE_STATUS, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n");

	return shader;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window)
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
	if (window == NULL)
	{
		PRINT_ERR("Failed to init window.");
		glfwTerminate();
	}

	_set_context_current(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
	glBufferData(GL_ARRAY_BUFFER, len * sizeof(vertices), vertices, GL_STATIC_DRAW);
}

static void _create_vao(unsigned int* vao)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
}

void _init_vertex_array32()
{
	// TODO: free memoty
	// TODO: resolve capacity thing
	g_VertexDataCapacity32 *= 2;
	VertexUnit32* unit_arr = (VertexUnit32*) realloc(VertexData32, g_VertexDataCapacity32 * sizeof(VertexUnit32));
	if (unit_arr == NULL)
	{
		// TODO: print err and terminate
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
	unit->shader_prog   = 0;
	for (int i = 0.f; i < 32; i++)
	{
		unit->data[i]   = 0.f;
	}

	g_UnitsNum32++;

	return unit;
}

VertexUnit32* _create_entry32(float* vertices, int len)
{
	if (VertexData32 == NULL)
	{
		_init_vertex_array32();
	}
	
	// TODO: assert len <= 32

	VertexUnit32* entry = _create_vertex_array32_entry();
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
	for (int i = 0; i < g_UnitsNum32; i++)
	{
		free(VertexData32 + i);
	}
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int draw_triangle(float* vertices[], int len)
{
	VertexUnit32* entry         = _create_entry32(vertices, len);

	unsigned int vertexShader   = _create_vertex_shader(k_vertexShaderSource);
	unsigned int fragmentShader = _create_fragment_shader(k_fragShaderSource);
	unsigned int shaderProg     = _compile_shader_program(&entry->shader_prog, vertexShader, fragmentShader);

	_create_vao(&entry->vao);
	_create_vbo(&entry->vbo, entry->data, len);

	// TODO: handle different attributes
	glVertexAttribPointer(0,                   // vertex attribute index
						  3,                   // size of vertex attribute (vec3)
		                  GL_FLOAT,            // data type
                          GL_FALSE,            // normalize?
		                  3 * sizeof(float),   // stride
		                  (void*)0             // position data offset
	                      );
	glEnableVertexAttribArray(0);
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
	while (!glfwWindowShouldClose(_window))
	{
		process_input(_window);

		glClearColor(g_BColor.R, g_BColor.G, g_BColor.B, g_BColor.A);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < g_UnitsNum32; i++)
		{
			VertexUnit32* entry = VertexData32 + i;
			glUseProgram(entry->shader_prog);
			glBindVertexArray(entry->vao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		glfwSwapBuffers(_window);
		glfwPollEvents();
	}

	_free_gl_resources();
	_free_entries32();

	glfwTerminate();
	
	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
