#include "map/initial_map.h"
#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "map/map_mgr.h"
#include "global_defs.h"
#include "graphics_defs.h"
#include "drawable_ops.h"

#define INTIAL_MAP_PATH_LEN 5

extern int wWidth;
extern int wHeight;

static PathSegment** s_Path = NULL;


// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int initial_add_background()
{
	DrawBufferData* draw_buf_data = NULL;
	get_quad_draw_buffer_data(&draw_buf_data);
	if (NULL == draw_buf_data)
	{
		PRINT_ERR("[entity]: Failed to retrieve square DrawBufferData.");
		return TERMINATE_ERR_CODE;
	}

	static const char* vertex_shader_path = "/res/static/shaders/basic_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/basic_frag.txt";
	static const char* texture_path = "/res/static/textures/field.jpg";

	DrawableDef* drawable = create_drawable();

	// TODO: Common code - move to separate function
	char path_buf[256];
	get_file_path(texture_path, &path_buf, 256);

	unsigned char* img_data;
	int width, height;
	fr_read_image_data(path_buf, &img_data, &width, &height);

	int create_texture_2D_res = create_texture_2D(img_data, width, height, &drawable->texture, TexType_RGB);
	fr_free_image_resources(img_data);

	if (TERMINATE_ERR_CODE == create_texture_2D_res)
	{
		PRINT_ERR("[static_env]: Failed to add env texute.");
		return TERMINATE_ERR_CODE;
	}

	int add_res = setup_drawable(drawable, draw_buf_data, vertex_shader_path, fragment_shader_path);
	if (TERMINATE_ERR_CODE == add_res)
	{
		PRINT_ERR("[static_env]: Failed to add env element.");
		return TERMINATE_ERR_CODE;
	}

	register_drawable_attribute(drawable, 3); // Pos
	register_drawable_attribute(drawable, 2); // Texture

	process_drawable_attributes(drawable);

	Vec3 translation = { { wWidth / 2.f, wHeight / 2.f, Z_DEPTH_INITIAL_MAP_BACKGROUND } };
	drawable->transform.translation = translation;

	Vec3 scale = { { wWidth / 2.f, wHeight / 2.f, 1.f } };
	drawable->transform.scale = scale;

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

	drawable->matrices.projection = COMMON_ORTHO_MAT;
    // TODO: Move "projection" into defs
	add_uniform_mat4f(drawable->shader_prog, COMMON_PROJECTION_UNIFORM_NAME, &drawable->matrices.projection);

	return 0;
}

int initial_add_path()
{
	static const char* vertex_shader_path = "/res/static/shaders/basic_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/basic_frag.txt";
	static const char* texture_path = "/res/static/textures/road.jpg";

	// TODO: Clear up "magic"
	float path_h = wHeight / 10;
	float half_path_h = path_h / 2.f;
	float path_y_offset = wHeight / 6;

	PathSegment predefined_path[INTIAL_MAP_PATH_LEN] = {
		{ .start = { 0.f, (float)(wHeight - path_y_offset) }, .end = { 550.f, (float)(wHeight - path_y_offset) } },
		{ .start = { 550.f, (float)(wHeight - path_y_offset) }, .end = { 550.f, (float)path_y_offset } },
		{ .start = { 550.f, (float)path_y_offset }, .end = { 1050.f, (float)path_y_offset } },
		{ .start = { 1050.f, (float)path_y_offset }, .end = { 1050.f, (float)wHeight / 2.f } },
		{ .start = { 1050.f, (float)wHeight / 2.f }, .end = { 1650.f, (float)wHeight / 2.f } }
	};

	PathSegment** path = malloc(INTIAL_MAP_PATH_LEN * sizeof *s_Path);
	if (NULL == path)
	{
		PRINT_ERR("[static_env]: Failed to create path ptr.");
		return TERMINATE_ERR_CODE;
	}

	assert((sizeof(predefined_path) / sizeof(predefined_path[0])) == INTIAL_MAP_PATH_LEN);
	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
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

	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
	{
		DrawBufferData* draw_buf_data = NULL;
		get_quad_draw_buffer_data(&draw_buf_data);
		if (NULL == draw_buf_data)
		{
			PRINT_ERR("[entity]: Failed to retrieve square DrawBufferData.");
			return TERMINATE_ERR_CODE;
		}

		DrawableDef* drawable = create_drawable();

		// TODO: Common code - move to separate function
		char path_buf[256];
		get_file_path(texture_path, &path_buf, 256);

		unsigned char* img_data;
		int width, height;
		fr_read_image_data(path_buf, &img_data, &width, &height);

		int create_texture_2D_res = create_texture_2D(img_data, width, height, &drawable->texture, TexType_RGB);
		fr_free_image_resources(img_data);

		if (TERMINATE_ERR_CODE == create_texture_2D_res)
		{
			PRINT_ERR("[static_env]: Failed to add initial map level\'s texute.");
			return TERMINATE_ERR_CODE;
		}

		int add_res = setup_drawable(drawable, draw_buf_data, vertex_shader_path, fragment_shader_path);
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

		Vec3 translation = { { pos_x, pos_y, Z_DEPTH_INITIAL_MAP_PATH } };
		drawable->transform.translation = translation;

		Vec3 scale = { { scale_x, scale_y, 1.f } };
		drawable->transform.scale = scale;

		drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

		drawable->matrices.projection = COMMON_ORTHO_MAT;
		add_uniform_mat4f(drawable->shader_prog, "projection", &drawable->matrices.projection);
	}

	return 0;
}

void initial_free_resources()
{
	if (NULL != s_Path)
	{
		for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
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
	return INTIAL_MAP_PATH_LEN;
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
