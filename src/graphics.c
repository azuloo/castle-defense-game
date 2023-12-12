#include "graphics.h"
#include "utils.h"
#include "file_reader.h"
#include "stb_image.h"
#include "lin_alg.h"

#include <stdlib.h>

#define PROGRAM_IV_LOG_BUF_CAPACITY 512

static GLFWwindow* window = NULL;
static InputFnPtr input_fn_ptr = NULL;

static int g_EntriesDataCapacity = 1;
static int g_EntriesNum = 0;
static EntryCnf* EntryCnfData = NULL;
static BackgroundColor g_BColor = { 0.2f, 0.3f, 0.3f, 1.0f };
static char* g_VertexShaderFilePath = "/res/vertex_source.TXT";
static char* g_FragShaderFilePath = "/res/fragment_source.TXT";

static void graphics_terminate(int code)
{
	graphics_free_resources();
	exit(code);
}

static void check_program_iv(unsigned int prog, unsigned int opcode, const char* msg)
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

static char* get_shader_source(const char* name)
{
	char vertex_source_path[256];
	get_file_path(name, vertex_source_path, 256);
	char* data_buf = '\0';
	size_t shader_size = 0;

	int res_code = readall(vertex_source_path, &data_buf, &shader_size);
	if (READ_OK != res_code)
	{
		PRINT_ERR_VARGS("Failed to load vertex source '%s', err code: %d.", name, res_code);
		graphics_terminate(TERMINATE_ERR_CODE);
	}

	return data_buf;
}

static void compile_shader_program(unsigned int* prog, unsigned int vertexShader, unsigned int fragShader)
{
	*prog = glCreateProgram();
	glAttachShader(*prog, vertexShader);
	glAttachShader(*prog, fragShader);
	glLinkProgram(*prog);

	check_program_iv(*prog, GL_LINK_STATUS, "Failed to link shader program: %s\n");

	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
}

static unsigned int create_vertex_shader(const char** source)
{
	unsigned int shader;

	shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader, 1, source, NULL);
	glCompileShader(shader);

	check_program_iv(shader, GL_COMPILE_STATUS, "Failed to compile vertex shader: %s\n");

	return shader;
}

static unsigned int create_fragment_shader(const char** source)
{
	unsigned int shader;

	shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader, 1, source, NULL);
	glCompileShader(shader);

	check_program_iv(shader, GL_COMPILE_STATUS, "Failed to compile fragment shader: %s\n");

	return shader;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void init_glfw()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, LIB_MAJOR_VER);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, LIB_MINOR_VER);
	glfwWindowHint(GLFW_OPENGL_PROFILE, LIB_DEFUALT_PROFILE);
}

static void set_context_current(GLFWwindow* window)
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

	set_context_current(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	return window;
}

static void init_glad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		PRINT_ERR("Failed to init GLAD.");
		glfwTerminate();
	}
}

static void set_viewport(int width, int height)
{
	glViewport(0, 0, width, height);
}

static void create_vbo(unsigned int* vbo, float* vertices, int len)
{
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * len, vertices, GL_STATIC_DRAW);
}

static void create_vao(unsigned int* vao)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
}

static void create_ebo(unsigned int* ebo, unsigned int* indices, int len)
{
	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * len, indices, GL_STATIC_DRAW);
}

static void alloc_entry_arr()
{
	// TODO: resolve capacity thing
	g_EntriesDataCapacity *= 2;
	EntryCnf* entries_arr = (EntryCnf*) realloc(EntryCnfData, sizeof(EntryCnf) * g_EntriesDataCapacity);
	if (NULL == entries_arr)
	{
		PRINT_ERR("Failed to allocate sufficient memory chunk for EntryCnf elements.");
		graphics_terminate(TERMINATE_ERR_CODE);
		return;
	}

	EntryCnfData = entries_arr;
}

static EntryCnf* create_entry_cnf()
{
	if (g_EntriesNum >= g_EntriesDataCapacity)
	{
		alloc_entry_arr();
	}

	EntryCnf* entry      = EntryCnfData + g_EntriesNum;
	entry->vbo           = 0;
	entry->vao           = 0;
	entry->ebo           = 0;
	entry->shader_prog   = 0;
	entry->texture       = 0;
	entry->num_indices   = 0;

	g_EntriesNum++;

	return entry;
}

static void free_gl_resources()
{
	for (int i = 0; i < g_EntriesNum; i++)
	{
		EntryCnf* entry = EntryCnfData + i;
		glDeleteVertexArrays(1, &entry->vao);
		glDeleteBuffers(1, &entry->vbo);
		glDeleteProgram(entry->shader_prog);
	}
}

static void free_entry_cnf_data()
{
	free(EntryCnfData);
}

static unsigned char* load_image(const char* path, int* width, int* height, int* nr_channels)
{
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load(path, width, height, nr_channels, 0);
	return data;
}

static void free_img_data(unsigned int* img_data)
{
	stbi_image_free(img_data);
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int graphics_should_be_terminated()
{
	return glfwWindowShouldClose(window);
}

void graphics_free_resources()
{
	free_gl_resources();
	free_entry_cnf_data();

	glfwTerminate();
}

EntryCnf* create_entry()
{
	if (NULL == EntryCnfData)
	{
		alloc_entry_arr();
	}

	EntryCnf* entry = create_entry_cnf();
	return entry;
}

void create_texture_2D(const char* img_path, unsigned int* texture)
{
	glGenTextures(1, texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *texture);
	// TODO: Configure filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nr_channels;
	unsigned char* data = load_image(img_path, &width, &height, &nr_channels);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		PRINT_ERR("Failed to load texture.");
	}
	free_img_data(data);
}

void add_uniform_mat4f(unsigned int shader_prog, const char* uniform_name, const Mat4* mat)
{
	glUseProgram(shader_prog);
	unsigned int transformLoc = glGetUniformLocation(shader_prog, uniform_name);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &mat[0].m[0]);
}

int add_element(EntryCnf* entry, DrawBufferData* buf_data)
{
	const char* vertex_shader_source = get_shader_source(g_VertexShaderFilePath);
	const char* fragment_shader_source = get_shader_source(g_FragShaderFilePath);

	unsigned int vertex_shader = create_vertex_shader(&vertex_shader_source);
	unsigned int fragment_shader = create_fragment_shader(&fragment_shader_source);

	compile_shader_program(&entry->shader_prog, vertex_shader, fragment_shader);

	create_vao(&entry->vao);
	create_vbo(&entry->vbo, buf_data->vertices, buf_data->vertices_len);
	create_ebo(&entry->ebo, buf_data->indices, buf_data->indices_len);
	entry->num_indices = buf_data->indices_len;

	// TODO: Move these out of here?
	glVertexAttribPointer(0,	// vertex attribute index
		3,						// size of vertex attribute
		GL_FLOAT,				// data type
		GL_FALSE,				// normalize?
		sizeof(float) * 8,		// stride
		(void*)0				// position data offset
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1,	     // vertex attribute index
		3,						     // size of vertex attribute
		GL_FLOAT,				     // data type
		GL_FALSE,				     // normalize?
		sizeof(float) * 8,		     // stride
		(void*)(sizeof(float) * 3)   // position data offset
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2,	     // vertex attribute index
		2,						     // size of vertex attribute
		GL_FLOAT,				     // data type
		GL_FALSE,				     // normalize?
		sizeof(float) * 8,		     // stride
		(void*)(sizeof(float) * 6)   // position data offset
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	return 0;
}

void close_window(GWindow* window)
{
	glfwSetWindowShouldClose(window, CLOSE_GLFW_WINDOW);
}

void bind_input_fn(InputFnPtr ptr)
{
	input_fn_ptr = ptr;
}

int init_graphics()
{
	// TODO: init_graphics() called before everything assertion (gl and GLAD)
	init_glfw();
	window = _create_window();
	init_glad();

	return 0;
}

void set_background_color(BackgroundColor b_color)
{
	g_BColor = b_color;
}

int draw()
{
	// TODO: Add check for NULL-ness
	(*input_fn_ptr)(window);

	glEnable(GL_DEPTH_TEST);
	glClearColor(g_BColor.R, g_BColor.G, g_BColor.B, g_BColor.A);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < g_EntriesNum; i++)
	{
		EntryCnf* entry = EntryCnfData + i;
		glUseProgram(entry->shader_prog);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, entry->texture);
		glBindVertexArray(entry->vao);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entry->ebo);
		glDrawElements(GL_TRIANGLES, entry->num_indices, GL_UNSIGNED_INT, 0);
	}

	glfwSwapBuffers(window);
	glfwPollEvents();
	
	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
