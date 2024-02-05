#include "map/initial_map.h"
#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "map/map_mgr.h"
#include "global_defs.h"
#include "graphics_defs.h"

extern int wWidth;
extern int wHeight;

static PathSegment** s_Path = NULL;
#define _INTIAL_MAP_PATH_LEN 5

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int initial_add_background()
{
	// TODO: Rect verices are identical; move somewhere
	static float vertices[] = {
		// Position            // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	static const char* vertex_shader_path = "/res/static/shaders/basic_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/basic_frag.txt";
	static const char* texture_path = "/res/static/textures/field.jpg";

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices) / sizeof(vertices[0]);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices) / sizeof(indices[0]);

	DrawableDef* drawable = create_drawable();

	char texture_buf[256];
	get_file_path(texture_path, &texture_buf, 256);

	int create_texture_2D_res = create_texture_2D(texture_buf, &drawable->texture, TexType_RGB);
	if (TERMINATE_ERR_CODE == create_texture_2D_res)
	{
		PRINT_ERR("[static_env]: Failed to add env texute.");
		return TERMINATE_ERR_CODE;
	}

	int add_res = config_drawable(drawable, &draw_buf_data, vertex_shader_path, fragment_shader_path);
	if (TERMINATE_ERR_CODE == add_res)
	{
		PRINT_ERR("[static_env]: Failed to add env element.");
		return TERMINATE_ERR_CODE;
	}

	register_drawable_attribute(drawable, 3); // Pos
	register_drawable_attribute(drawable, 2); // Texture

	process_drawable_attributes(drawable);

	drawable->matrices->model = IdentityMat;
	scale(&drawable->matrices->model, wWidth / 2.f, wHeight / 2.f, 1.f);
	translate(&drawable->matrices->model, wWidth / 2.f, wHeight / 2.f, Z_DEPTH_INITIAL_MAP_BACKGROUND);
	add_uniform_mat4f(drawable->shader_prog, "model", &drawable->matrices->model);

	drawable->matrices->projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(drawable->shader_prog, "projection", &drawable->matrices->projection);

	return 0;
}

int initial_add_path()
{
	// TODO: Rect verices are identical; move somewhere
	static float vertices[] = {
		// Position            // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	// TODO: Rect indices are identical; move somewhere
	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	static const char* vertex_shader_path = "/res/static/shaders/basic_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/basic_frag.txt";
	static const char* texture_path = "/res/static/textures/road.jpg";

	// TODO: Clear up "magic"
	float path_h = wHeight / 10;
	float half_path_h = path_h / 2.f;
	float path_y_offset = wHeight / 6;

	PathSegment predefined_path[_INTIAL_MAP_PATH_LEN] = {
		{ .start = { 0.f, (float)(wHeight - path_y_offset) }, .end = { 550.f, (float)(wHeight - path_y_offset) } },
		{ .start = { 550.f, (float)(wHeight - path_y_offset) }, .end = { 550.f, (float)path_y_offset } },
		{ .start = { 550.f, (float)path_y_offset }, .end = { 1050.f, (float)path_y_offset } },
		{ .start = { 1050.f, (float)path_y_offset }, .end = { 1050.f, (float)wHeight / 2.f } },
		{ .start = { 1050.f, (float)wHeight / 2.f }, .end = { 1400.f, (float)wHeight / 2.f } }
	};

	PathSegment** path = malloc(_INTIAL_MAP_PATH_LEN * sizeof *s_Path);
	if (NULL == path)
	{
		PRINT_ERR("[static_env]: Failed to create path ptr.");
		return TERMINATE_ERR_CODE;
	}

	assert((sizeof(predefined_path) / sizeof(predefined_path[0])) == _INTIAL_MAP_PATH_LEN);
	for (int i = 0; i < _INTIAL_MAP_PATH_LEN; i++)
	{
		PathSegment* path_seg = malloc(sizeof *path_seg);
		if (NULL == path_seg)
		{
			PRINT_ERR("[static_env]: Failed to create path segment ptr.");
			return TERMINATE_ERR_CODE;
		}

		path[i] = path_seg;
		*path[i] = predefined_path[i];
	}

	s_Path = path;

	for (int i = 0; i < _INTIAL_MAP_PATH_LEN; i++)
	{
		DrawBufferData draw_buf_data;
		draw_buf_data.vertices = vertices;
		draw_buf_data.vertices_len = sizeof(vertices) / sizeof(vertices[0]);
		draw_buf_data.indices = indices;
		draw_buf_data.indices_len = sizeof(indices) / sizeof(indices[0]);

		DrawableDef* drawable = create_drawable();

		char texture_buf[256];
		get_file_path(texture_path, &texture_buf, 256);

		int create_texture_2D_res = create_texture_2D(texture_buf, &drawable->texture, TexType_RGB);
		if (TERMINATE_ERR_CODE == create_texture_2D_res)
		{
			PRINT_ERR("[static_env]: Failed to add initial map level\'s texute.");
			return TERMINATE_ERR_CODE;
		}

		int add_res = config_drawable(drawable, &draw_buf_data, vertex_shader_path, fragment_shader_path);
		if (TERMINATE_ERR_CODE == add_res)
		{
			PRINT_ERR("[static_env]: Failed to add initial map level\'s element.");
			return TERMINATE_ERR_CODE;
		}

		register_drawable_attribute(drawable, 3); // Pos
		register_drawable_attribute(drawable, 2); // Texture

		process_drawable_attributes(drawable);

		PathSegment* path_segment = s_Path[i];

		float pos_x_diff = path_segment->end.x - path_segment->start.x;
		float pos_y_diff = path_segment->end.y - path_segment->start.y;

		float scale_x = 0;
		float scale_y = 0;

		if (pos_x_diff == 0)
		{
			if (pos_y_diff < 0)
			{
				scale_y = pos_y_diff / 2.f - half_path_h;
			}
			else
			{
				scale_y = pos_y_diff / 2.f + half_path_h;
			}

			scale_x = half_path_h;
		}
		else if (pos_y_diff == 0)
		{
			if (pos_x_diff < 0)
			{
				scale_x = pos_x_diff / 2.f - half_path_h;
			}
			else
			{
				scale_x = pos_x_diff / 2.f + half_path_h;
			}

			scale_y = half_path_h;
		}

		float pos_x = (path_segment->start.x + path_segment->end.x) / 2.f;
		float pos_y = (path_segment->start.y + path_segment->end.y) / 2.f;

		drawable->matrices->model = IdentityMat;
		scale(&drawable->matrices->model, scale_x, scale_y, 1.f);
		translate(&drawable->matrices->model, pos_x, pos_y, Z_DEPTH_INITIAL_MAP_PATH);
		add_uniform_mat4f(drawable->shader_prog, "model", &drawable->matrices->model);

		drawable->matrices->projection = COMMON_ORTHO_MAT;
		add_uniform_mat4f(drawable->shader_prog, "projection", &drawable->matrices->projection);
	}

	return 0;
}

void initial_free_resources()
{
	if (NULL != s_Path)
	{
		for (int i = 0; i < _INTIAL_MAP_PATH_LEN; i++)
		{
			free(s_Path[i]);
		}

		free(s_Path);
	}
}

const PathSegment** get_initial_path()
{
	return s_Path;
}

int get_initial_path_len()
{
	return _INTIAL_MAP_PATH_LEN;
}

int initial_map_init()
{
	MapFuncsDef* map_func_def = malloc(sizeof * map_func_def);
	if (NULL == map_func_def)
	{
		PRINT_ERR("[initial_map]: Failed to allocate sufficient memory for MapFuncsDef.");
		return TERMINATE_ERR_CODE;
	}

	map_func_def->add_background    = initial_add_background;
	map_func_def->add_path          = initial_add_path;
	map_func_def->free_resources    = initial_free_resources;

	map_func_def->get_path          = get_initial_path;
	map_func_def->get_path_len      = get_initial_path_len;

	map_mgr_register_map(map_func_def);

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
