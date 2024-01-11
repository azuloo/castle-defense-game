#include "graphics.h"
#include "file_reader.h"
#include "stb_image.h"
#include "lin_alg.h"

#include <stdlib.h>

#define PROGRAM_IV_LOG_BUF_CAPACITY 512

static GLFWwindow* window = NULL;
static InputFnPtr input_fn_ptr = NULL;
static WindowResizeFnPtr window_resize_fn_ptr = NULL;

static int g_EntriesDataCapacity = 1;
static int g_EntriesNum = 0;
static EntryCnf* EntryCnfData = NULL;
static BackgroundColor g_BColor = { 0.f, 0.f, 0.f, 1.f };

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

// ! Allocates memory on heap (indirect) !
static char* get_shader_source(const char* name)
{
	char vertex_source_path[256];
	get_file_path(name, vertex_source_path, 256);
	char* data_buf = '\0';
	size_t shader_size = 0;

	int res_code = readall(vertex_source_path, &data_buf, &shader_size);
	if (READ_OK != res_code)
	{
		PRINT_ERR_VARGS("[graphics]: Failed to load vertex source '%s', err code: %d.", name, res_code);
		return NULL;
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
	if (NULL != window_resize_fn_ptr)
	{
		(*window_resize_fn_ptr)(window, width, height);
	}
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

static GLFWwindow* create_window()
{
	GLFWwindow* window = glfwCreateWindow(WINDOW_DEFAULT_RES_W, WINDOW_DEFAULT_RES_H, WINDOW_DEFUALT_NAME, NULL, NULL);
	if (NULL == window)
	{
		PRINT_ERR("[graphics]: Failed to init window.");
		return NULL;
	}
	set_context_current(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	return window;
}

static void init_glad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		PRINT_ERR("[graphics]: Failed to init GLAD.");
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

// ! Allocates memory on heap !
static int alloc_entry_arr()
{
	g_EntriesDataCapacity *= 2;
	EntryCnf* entries_arr = (EntryCnf*) realloc(EntryCnfData, sizeof(EntryCnf) * g_EntriesDataCapacity);
	if (NULL == entries_arr)
	{
		PRINT_ERR("[graphics]: Failed to allocate sufficient memory chunk for EntryCnf elements.");
		return TERMINATE_ERR_CODE;
	}
	EntryCnfData = entries_arr;
}

static int add_entry_attributes_cnf(EntryCnf* entry)
{
	entry->attributes->capacity *= 2;
	AttributeCnf* attr_cnf = (AttributeCnf*) realloc(entry->attributes->elements, entry->attributes->capacity * sizeof(AttributeCnf));
	if (NULL == attr_cnf)
	{
		PRINT_ERR("[graphics]: Failed to allocate sufficient memory chunk for AttributeCnf elements.");
		return TERMINATE_ERR_CODE;
	}
	entry->attributes->elements = attr_cnf;
	for (int i = entry->attributes->count; i < entry->attributes->capacity; i++)
	{
		AttributeCnf* attr_cnf = entry->attributes->elements + i;
		attr_cnf->idx = 0;
		attr_cnf->size = 0;
	}
}

static int create_entry_attributes(EntryCnf* entry)
{
	entry->attributes = (GAttributes*) malloc(sizeof(GAttributes));
	if (NULL == entry->attributes)
	{
		PRINT_ERR("[graphics]: Failed to allocate sufficient memory chunk for GAttributes elements.");
		return TERMINATE_ERR_CODE;
	}

	entry->attributes->elements   = NULL;
	entry->attributes->type       = GL_FLOAT;
	entry->attributes->normalize  = GL_FALSE;
	entry->attributes->stride     = 0;
	entry->attributes->capacity   = 1;
	entry->attributes->count      = 0;

	int add_entry_attributes_cnf_res = add_entry_attributes_cnf(entry);
	return add_entry_attributes_cnf_res;
}

static EntryCnf* create_entry_cnf()
{
	if (g_EntriesNum >= g_EntriesDataCapacity)
	{
		int alloc_entry_res = alloc_entry_arr();
		if (TERMINATE_ERR_CODE == alloc_entry_res)
		{
			PRINT_ERR("[graphics]: Failed to create entry config.");
			return NULL;
		}
	}

	EntryCnf* entry      = EntryCnfData + g_EntriesNum;
	entry->vbo           = 0;
	entry->vao           = 0;
	entry->ebo           = 0;
	entry->shader_prog   = 0;
	entry->texture       = 0;
	entry->num_indices   = 0;
	entry->attributes    = NULL;

	int create_entry_attributes_res = create_entry_attributes(entry);
	if (TERMINATE_ERR_CODE == create_entry_attributes_res)
	{
		PRINT_ERR("[graphics]: Failed to create entry attibutes.");
		return NULL;
	}

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

static void free_entry_attributes_cnf()
{
	for (int i = 0; i < g_EntriesNum; i++)
	{
		EntryCnf* entry = EntryCnfData + i;
		if (NULL == entry->attributes)
		{
			continue;
		}

		free(entry->attributes->elements);
		free(entry->attributes);
	}
}

static void free_entry_cnf_data()
{
	free_entry_attributes_cnf();
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
	// TODO: Check for NULL and return error
	if (NULL == EntryCnfData)
	{
		alloc_entry_arr();
	}

	EntryCnf* entry = create_entry_cnf();
	return entry;
}

int create_texture_2D(const char* img_path, unsigned int* texture, enum TextureType type)
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
	if (NULL == data)
	{
		PRINT_ERR("[graphics]: Failed to load texture.");
		return TERMINATE_ERR_CODE;
	}

	switch (type)
	{
	case TexType_RGB:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		break;
	case TexType_RGBA:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	default:
		break;
	}

	glGenerateMipmap(GL_TEXTURE_2D);
	free_img_data(data);

	return 0;
}

int add_uniform_mat4f(unsigned int shader_prog, const char* uniform_name, const Mat4* mat)
{
	glUseProgram(shader_prog);
	unsigned int transformLoc = glGetUniformLocation(shader_prog, uniform_name);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &mat->m[0]);
	return 0;
}

int add_element(EntryCnf* entry, DrawBufferData* buf_data, const char* vertex_shader_path, const char* fragment_shader_path)
{
	const char* vertex_shader_source = get_shader_source(vertex_shader_path);
	const char* fragment_shader_source = get_shader_source(fragment_shader_path);

	if (NULL == vertex_shader_source || NULL == fragment_shader_source)
	{
		return TERMINATE_ERR_CODE;
	}

	unsigned int vertex_shader = create_vertex_shader(&vertex_shader_source);
	unsigned int fragment_shader = create_fragment_shader(&fragment_shader_source);

	free(vertex_shader_source);
	free(fragment_shader_source);

	compile_shader_program(&entry->shader_prog, vertex_shader, fragment_shader);

	create_vao(&entry->vao);
	create_vbo(&entry->vbo, buf_data->vertices, buf_data->vertices_len);
	create_ebo(&entry->ebo, buf_data->indices, buf_data->indices_len);
	entry->num_indices = buf_data->indices_len;

	return 0;
}

int add_entry_attribute(EntryCnf* entry, unsigned int size)
{
	GAttributes* attributes = entry->attributes;
	if (attributes->count == attributes->capacity)
	{
		add_entry_attributes_cnf(entry);
	}

	AttributeCnf* attr_cnf = attributes->elements + attributes->count;
	attr_cnf->idx  = attributes->count;
	attr_cnf->size = size;

	attributes->stride += size;
	attributes->count  += 1;

	return 0;
}

int apply_entry_attributes(EntryCnf* entry)
{
	glBindVertexArray(entry->vao);
	GAttributes* attributes = entry->attributes;
	unsigned int offset = 0;
	for (int i = 0; i < attributes->count; i++)
	{
		AttributeCnf* attr_cnf = attributes->elements + i;
		glVertexAttribPointer(attr_cnf->idx,	           // vertex attribute index
			attr_cnf->size,						           // size of vertex attribute
			attributes->type,				               // data type
			attributes->normalize,				           // normalize?
			sizeof(float) * attributes->stride,		       // stride
			(void*)(sizeof(float) * offset)				   // position data offset
		);
		glEnableVertexAttribArray(attr_cnf->idx);
		offset += attr_cnf->size;
	}
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

void bind_window_resize_fn(WindowResizeFnPtr ptr)
{
	window_resize_fn_ptr = ptr;
}

int init_graphics()
{
	// TODO: init_graphics() called before everything assertion (gl and GLAD)
	init_glfw();
	window = create_window();
	if (NULL == window)
	{
		PRINT_ERR("[graphics]: Failed to create window.");
		return TERMINATE_ERR_CODE;
	}
	init_glad();

	return 0;
}

void set_background_color(BackgroundColor b_color)
{
	g_BColor = b_color;
}

int draw()
{
	if (NULL != input_fn_ptr)
	{
		(*input_fn_ptr)(window);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glClearColor(g_BColor.R, g_BColor.G, g_BColor.B, g_BColor.A);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
