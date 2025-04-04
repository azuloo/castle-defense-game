#include "graphics.h"
#include "file_reader.h"
#include "stb_image.h"
#include "lin_alg.h"
#include "utils.h"

#include <stdlib.h>

#define PROGRAM_IV_LOG_BUF_CAPACITY 512

static int s_Initialized                        = 0;

static GLFWwindow* s_Window                     = NULL;
static InputFnPtr input_fn_ptr                  = NULL;
static WindowResizeFnPtr window_resize_fn_ptr   = NULL;

static int s_DrawableDataCapacity       = 32;
static int s_DrawableNum                = 0;
static DrawableDef* s_DrawableData      = NULL;

// For DrawableDef handlers mapping
static int* s_HandleToIdx               = NULL;
static int* s_IdxToHandle               = NULL;

static BackgroundColor s_BColor         = { 0.f, 0.f, 0.f, 1.f };

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
	static const float targetAr = 16.f / 9.f;
	float windowAr = (float)width / (float)height;
	float resAr = windowAr / targetAr;

	int resWidth = width;
	int resHeight = height;
	int x = 0;
	int y = 0;

	if (resAr < 1.f)
	{
		resHeight = resWidth * 9 / 16;
		y = (height - resHeight) / 2;
	}
	else
	{
		resWidth = resHeight * 16 / 9;
		x = (width - resWidth) / 2;
	}

	glViewport(x, y, resWidth, resHeight);

	if (NULL != window_resize_fn_ptr)
	{
		(*window_resize_fn_ptr)(window, x, y, resWidth, resHeight);
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
	CHECK_EXPR_FAIL_RET_NULL(NULL != window, "[graphics]: Failed to init window.");

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

// Graphics sub-module functions

int create_vbo(unsigned int* vbo, float* vertices, int len, int draw_mode)
{
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, len * sizeof * vertices, vertices, draw_mode);

	return 0;
}

int create_vao(unsigned int* vao)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	return 0;
}

int create_ebo(unsigned int* ebo, unsigned int* indices, int len, int draw_mode)
{
	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * len, indices, draw_mode);

	return 0;
}

int compile_shaders(unsigned int* shader_prog, const char* vertex_shader_src, const char* fragment_shader_src)
{
	unsigned int vertex_shader = create_vertex_shader(&vertex_shader_src);
	unsigned int fragment_shader = create_fragment_shader(&fragment_shader_src);

	compile_shader_program(shader_prog, vertex_shader, fragment_shader);

	return 0;
}

// ! Allocates memory on heap (indirect) !
char* get_shader_source(const char* name)
{
	char vertex_source_path[256];
	get_file_path(name, &vertex_source_path, 256);
	char* data_buf = '\0';
	size_t shader_size = 0;

	int res_code = readall(vertex_source_path, &data_buf, &shader_size);
	if (READ_OK != res_code)
	{
		PRINT_ERR_VARGS("[graphics]: Failed to load vertex source '%s', err code: %d.", name, res_code);
		return NULL;
	}

	// TODO: Work throug return type (char*)
	return data_buf;
}

// ! Allocates memory on heap !
int create_drawable_buffer_data(DrawableDef* drawable, DrawBufferData* src)
{
	if (NULL != src->vertices && src->vertices_len != 0)
	{
		float* vertices = malloc(src->vertices_len * sizeof * vertices);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != vertices, "[graphics] Failed to allocate sufficient memory for buffer_data vertices.");

		drawable->buffer_data.vertices = vertices;
		drawable->buffer_data.vertices_len = src->vertices_len;

		memcpy(drawable->buffer_data.vertices, src->vertices, src->vertices_len * sizeof * src->vertices);
	}

	if (NULL != src->indices && src->indices_len != 0)
	{
		int* indices = malloc(src->indices_len * sizeof * indices);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != indices, "[graphics] Failed to allocate sufficient memory for buffer_data indices.");

		drawable->buffer_data.indices = indices;
		drawable->buffer_data.indices_len = src->indices_len;

		memcpy(drawable->buffer_data.indices, src->indices, src->indices_len * sizeof * src->indices);
	}

	return 0;
}

// Graphics sub-module functions end

// ! Allocates memory on heap !
static int alloc_drawable_arr()
{
	s_DrawableDataCapacity *= 2;
	DrawableDef* drawable_arr = realloc(s_DrawableData, s_DrawableDataCapacity * sizeof *drawable_arr);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable_arr, "[graphics]: Failed to allocate sufficient memory chunk for DrawableDef arr.");

	s_DrawableData = drawable_arr;

	int* handle_to_idx_arr = realloc(s_HandleToIdx, s_DrawableDataCapacity * sizeof *handle_to_idx_arr);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != handle_to_idx_arr, "[graphics]: Failed to allocate memory for s_HandleToIdx arr.");
	
	s_HandleToIdx = handle_to_idx_arr;

	int* idx_to_handle_arr = realloc(s_IdxToHandle, s_DrawableDataCapacity * sizeof *idx_to_handle_arr);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != idx_to_handle_arr, "[graphics]: Failed to allocate memory for s_IdxToHandle arr.");
	
	s_IdxToHandle = idx_to_handle_arr;

	return 0;
}

static int add_drawable_attributes(DrawableDef* drawable)
{
	drawable->attributes->capacity *= 2;
	AttributeCnf* attr_cnf = realloc(drawable->attributes->elements, drawable->attributes->capacity * sizeof *attr_cnf);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != attr_cnf, "[graphics]: Failed to allocate sufficient memory chunk for AttributeCnf elements.");

	drawable->attributes->elements = attr_cnf;
	for (unsigned int i = drawable->attributes->count; i < drawable->attributes->capacity; i++)
	{
		AttributeCnf* attr_cnf = drawable->attributes->elements + i;
		attr_cnf->idx = 0;
		attr_cnf->size = 0;
	}

	return 0;
}

static int create_drawable_attributes(DrawableDef* drawable)
{
	drawable->attributes = malloc(sizeof *drawable->attributes);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable->attributes, "[graphics]: Failed to allocate sufficient memory chunk for GAttributes elements.");

	drawable->attributes->elements   = NULL;
	drawable->attributes->type       = GL_FLOAT;
	drawable->attributes->normalize  = GL_FALSE;
	drawable->attributes->stride     = 0;
	drawable->attributes->capacity   = 1;
	drawable->attributes->count      = 0;

	int add_drawable_attributes_res = add_drawable_attributes(drawable);

	return add_drawable_attributes_res;
}

static DrawableDef* create_drawable_def()
{
	// TODO: How do we solve pointers invalidation problem? (use Registry)
	if (s_DrawableNum >= s_DrawableDataCapacity)
	{
		int alloc_drawable_res = alloc_drawable_arr();
		CHECK_EXPR_FAIL_RET_NULL(TERMINATE_ERR_CODE != alloc_drawable_res, "[graphics]: Failed to create drawable def.");
	}

	DrawableDef* drawable             = s_DrawableData + s_DrawableNum;
	drawable->buffer_data.vertices    = NULL;
	drawable->buffer_data.indices     = NULL;
	drawable->shader_prog             = 0;
	drawable->texture                 = 0;
	drawable->num_indices             = 0;
	drawable->attributes              = NULL;
	drawable->handle                  = s_DrawableNum;
	drawable->visible                 = 1;
	drawable->draw_mode               = DRAW_MODE_STATIC;

	drawable->draw_layer              = 0;

	memset(&drawable->buffers, 0, sizeof * &drawable->buffers);
	memset(&drawable->matrices, 0, sizeof * &drawable->matrices);
	memset(&drawable->transform, 0, sizeof * &drawable->transform);
	memset(&drawable->init_transform, 0, sizeof * &drawable->init_transform);

	int create_drawable_attributes_res = create_drawable_attributes(drawable);
	CHECK_EXPR_FAIL_RET_NULL(TERMINATE_ERR_CODE != create_drawable_attributes_res, "[graphics]: Failed to create drawable attibutes.");

	s_HandleToIdx[s_DrawableNum] = s_DrawableNum;
	s_IdxToHandle[s_DrawableNum] = s_DrawableNum;

	s_DrawableNum++;

	return drawable;
}

static void free_gl_resources()
{
	for (int i = 0; i < s_DrawableNum; i++)
	{
		DrawableDef* drawable = s_DrawableData + i;
		glDeleteVertexArrays(1, &drawable->buffers.vao);
		glDeleteBuffers(1, &drawable->buffers.vbo);
		glDeleteProgram(drawable->shader_prog);
	}
}

static void free_drawable_attributes_cnf()
{
	for (int i = 0; i < s_DrawableNum; i++)
	{
		DrawableDef* drawable = s_DrawableData + i;
		if (NULL == drawable->attributes)
		{
			continue;
		}

		free(drawable->attributes->elements);
		free(drawable->attributes);
	}
}

static void free_drawable_buffer_data()
{
	for (int i = 0; i < s_DrawableNum; i++)
	{
		DrawableDef* drawable = s_DrawableData + i;
		if (NULL != drawable->buffer_data.vertices)
		{
			free(drawable->buffer_data.vertices);
		}

		if (NULL != drawable->buffer_data.indices)
		{
			free(drawable->buffer_data.indices);
		}
	}
}

static void free_drawable_data()
{
	free_drawable_attributes_cnf();
	free_drawable_buffer_data();
	free(s_DrawableData);
	free(s_HandleToIdx);
	free(s_IdxToHandle);
}

int graphics_get_cursor_pos(double* xpos, double* ypos)
{
	ASSERT_GRAPHICS_INITIALIZED

	glfwGetCursorPos(s_Window, xpos, ypos);
	return 0;
}

int check_graphics_initialized()
{
	return s_Initialized;
}

int graphics_should_be_terminated()
{
	ASSERT_GRAPHICS_INITIALIZED

	return glfwWindowShouldClose(s_Window);
}

void graphics_free_resources()
{
	ASSERT_GRAPHICS_INITIALIZED

	free_gl_resources();
	free_drawable_data();

	glfwTerminate();
}

void set_unpack_alignment(int align)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, align);
}

int create_drawable(DrawableDef** dest)
{
	ASSERT_GRAPHICS_INITIALIZED

	CHECK_EXPR_FAIL_RET_TERMINATE(*dest == NULL, "[graphics]: Destination drawable must be empty.");

	if (NULL == s_DrawableData)
	{
		int alloc_drawable_res = alloc_drawable_arr();
		CHECK_EXPR_FAIL_RET_NULL(TERMINATE_ERR_CODE != alloc_drawable_res, "[graphics]: Failed to create drawable.");
	}

	DrawableDef* drawable = create_drawable_def();
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[graphics]: Failed to create drawable.");

	*dest = drawable;

	return 0;
}

int create_texture_2D(unsigned char* data, int width, int height, unsigned int* texture, enum TextureType type, int* tex_params, int tex_params_amount)
{
	ASSERT_GRAPHICS_INITIALIZED

	glGenTextures(1, texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *texture);
	CHECK_EXPR_FAIL_RET_TERMINATE(tex_params_amount % 2 == 0, "[graphics]: Texture parameters should come in paris (parameter-value).");
	for (int i = 0; i < tex_params_amount; i += 2)
	{
		int param_name = tex_params[i];
		int param_value = tex_params[i+1];
		glTexParameteri(GL_TEXTURE_2D, param_name, param_value);
	}

	switch (type)
	{
	case TexType_RGB:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		break;
	case TexType_RGBA:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	case TexType_RED:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		break;
	default:
		break;
	}

	glGenerateMipmap(GL_TEXTURE_2D);

	return 0;
}

int add_uniform_mat4f(unsigned int shader_prog, const char* uniform_name, const Mat4* mat)
{
	ASSERT_GRAPHICS_INITIALIZED

	glUseProgram(shader_prog);
	unsigned int transformLoc = glGetUniformLocation(shader_prog, uniform_name);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &mat->m[0]);

	return 0;
}

int add_uniform_vec4f(unsigned int shader_prog, const char* uniform_name, const Vec4* vec)
{
	ASSERT_GRAPHICS_INITIALIZED

	glUseProgram(shader_prog);
	unsigned int transformLoc = glGetUniformLocation(shader_prog, uniform_name);
	glUniform4f(transformLoc, vec->x, vec->y, vec->z, vec->w);

	return 0;
}

int add_uniform_vec3f(unsigned int shader_prog, const char* uniform_name, const Vec3* vec)
{
	ASSERT_GRAPHICS_INITIALIZED

	glUseProgram(shader_prog);
	unsigned int transformLoc = glGetUniformLocation(shader_prog, uniform_name);
	glUniform3f(transformLoc, vec->x, vec->y, vec->z);

	return 0;
}

int add_uniform_1f(unsigned int shader_prog, const char* uniform_name, float val)
{
	ASSERT_GRAPHICS_INITIALIZED

	glUseProgram(shader_prog);
	unsigned int transformLoc = glGetUniformLocation(shader_prog, uniform_name);
	glUniform1f(transformLoc, val);

	return 0;
}

int add_texture_2D(DrawableDef* drawable, const char* texture_path, int texture_type, int texture_params[], int texture_params_count)
{
	static char path_buf[256];
	get_file_path(texture_path, &path_buf, 256);

	unsigned char* img_data;
	int width, height;
	fr_read_image_data(path_buf, &img_data, &width, &height);

	memset(path_buf, 0, sizeof *path_buf);

	int create_texture_2D_res = create_texture_2D(img_data, width, height, &drawable->texture, texture_type, texture_params, texture_params_count);
	fr_free_image_resources(img_data);
	CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != create_texture_2D_res, "[graphics]: Failed to add env texute.");

	return 0;
}

int setup_drawable(DrawableDef* drawable, const DrawBufferData* buf_data, const char* vertex_shader_path, const char* fragment_shader_path)
{
	ASSERT_GRAPHICS_INITIALIZED

	char* vertex_shader_src = get_shader_source(vertex_shader_path);
	char* fragment_shader_src = get_shader_source(fragment_shader_path);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != vertex_shader_src && NULL != fragment_shader_src, "[graphics] Failed to get vertex and/or fragment source.");

	compile_shaders(&drawable->shader_prog, vertex_shader_src, fragment_shader_src);

	create_vao(&drawable->buffers.vao);
	create_vbo(&drawable->buffers.vbo, buf_data->vertices, buf_data->vertices_len, drawable->draw_mode);
	create_ebo(&drawable->buffers.ebo, buf_data->indices, buf_data->indices_len, drawable->draw_mode);
	drawable->num_indices = buf_data->indices_len;

	free(vertex_shader_src);
	free(fragment_shader_src);

	return 0;
}

int register_drawable_attribute(DrawableDef* drawable, unsigned int size)
{
	ASSERT_GRAPHICS_INITIALIZED

	GAttributes* attributes = drawable->attributes;
	if (attributes->count == attributes->capacity)
	{
		add_drawable_attributes(drawable);
	}

	AttributeCnf* attr_cnf = attributes->elements + attributes->count;
	attr_cnf->idx  = attributes->count;
	attr_cnf->size = size;

	attributes->stride += size;
	attributes->count  += 1;

	return 0;
}

int process_drawable_attributes(DrawableDef* drawable)
{
	ASSERT_GRAPHICS_INITIALIZED

	glBindVertexArray(drawable->buffers.vao);
	GAttributes* attributes = drawable->attributes;
	unsigned int offset = 0;
	for (unsigned int i = 0; i < attributes->count; i++)
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
	ASSERT_GRAPHICS_INITIALIZED

	glfwSetWindowShouldClose(window, CLOSE_GLFW_WINDOW);
}

void bind_input_fn(InputFnPtr ptr)
{
	ASSERT_GRAPHICS_INITIALIZED

	input_fn_ptr = ptr;
}

void bind_window_resize_fn(WindowResizeFnPtr ptr)
{
	ASSERT_GRAPHICS_INITIALIZED

	window_resize_fn_ptr = ptr;
}

void bind_key_pressed_cb(KeyCbPtr ptr)
{
	ASSERT_GRAPHICS_INITIALIZED

	glfwSetKeyCallback(s_Window, ptr);
}

void bind_mouse_button_cb(MouseButtonCbPtr ptr)
{
	ASSERT_GRAPHICS_INITIALIZED

	glfwSetMouseButtonCallback(s_Window, ptr);
}

int init_graphics()
{
	init_glfw();
	s_Window = create_window();
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_Window, "[graphics]: Failed to create window.");

	init_glad();

	s_Initialized = 1;

	return 0;
}

void set_background_color(BackgroundColor b_color)
{
	ASSERT_GRAPHICS_INITIALIZED

	s_BColor = b_color;
}

void drawable_set_visible(DrawableDef* drawable, int visible)
{
	drawable->visible = visible;
}

int get_drawable_def(DrawableDef** dest, int handle)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_DrawableData, "[graphics]: Drawable arr has not been initialized.");
	CHECK_EXPR_FAIL_RET_TERMINATE(handle >= 0 && handle < s_DrawableNum, "[graphics]: Drawable handle is out of bounds.");

	int idx = s_HandleToIdx[handle];
	DrawableDef* drawable = s_DrawableData + idx;
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[graphics]: Drawable has not been set.");

	*dest = drawable;

	return 0;
}

static void swap_drawables(int f, int s)
{
	DrawableDef f_tmp = s_DrawableData[f];

	s_DrawableData[f] = s_DrawableData[s];
	s_DrawableData[s] = f_tmp;

	int handle_f = s_IdxToHandle[f];
	int handle_s = s_IdxToHandle[s];

	s_IdxToHandle[f] = handle_s;
	s_IdxToHandle[s] = handle_f;

	s_HandleToIdx[handle_f] = s;
	s_HandleToIdx[handle_s] = f;
}

static int partition_drawables(int low, int high)
{
	int pivot = s_DrawableData[high].draw_layer;
	int i = low - 1;

	for (int j = low; j < high; j++)
	{
		if (s_DrawableData[j].draw_layer <= pivot)
		{
			i++;
			swap_drawables(i, j);
		}
	}

	swap_drawables(i + 1, high);
	return i + 1;
}

static void quicksort_drawables(int low, int high)
{
	if (low < high)
	{
		int p = partition_drawables(low, high);
		quicksort_drawables(low, p - 1);
		quicksort_drawables(p + 1, high);
	}
}

void sort_drawables()
{
	quicksort_drawables(0, s_DrawableNum - 1);
}

int graphics_draw()
{
	ASSERT_GRAPHICS_INITIALIZED

	if (NULL != input_fn_ptr)
	{
		(*input_fn_ptr)(s_Window);
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(s_BColor.R, s_BColor.G, s_BColor.B, s_BColor.A);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < s_DrawableNum; i++)
	{
		DrawableDef* drawable = s_DrawableData + i;
		if (!drawable->visible)
			continue;
		glUseProgram(drawable->shader_prog);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(drawable->buffers.vao);
		glBindTexture(GL_TEXTURE_2D, drawable->texture);

		if (DRAW_MODE_STATIC == drawable->draw_mode)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable->buffers.ebo);
			glDrawElements(GL_TRIANGLES, drawable->num_indices, GL_UNSIGNED_INT, 0);
		}

		if (DRAW_MODE_DYNAMIC == drawable->draw_mode)
		{
			glBindBuffer(GL_ARRAY_BUFFER, drawable->buffers.vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, drawable->buffer_data.vertices_len * sizeof * drawable->buffer_data.vertices, drawable->buffer_data.vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, drawable->buffer_data.vertices_len / 4); // TODO: Remove magic number
		}
	}

	glfwSwapBuffers(s_Window);
	glfwPollEvents();
	
	return 0;
}
