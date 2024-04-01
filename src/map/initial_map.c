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
	// TODO: Replace with quad_draw()
	DrawBufferData* draw_buf_data = NULL;
	get_quad_draw_buffer_data(&draw_buf_data);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != draw_buf_data, "[initial_map]: Failed to retrieve square DrawBufferData.");

	static const char* vertex_shader_path = "/res/static/shaders/basic_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/basic_frag.txt";
	static const char* texture_path = "/res/static/textures/field.jpg";

	DrawableDef* drawable = NULL;
	create_drawable(&drawable);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[initial_map]: Drawable is not defined.");

	add_texture_2D(drawable, texture_path, TexType_RGB);

	int add_res = setup_drawable(drawable, draw_buf_data, vertex_shader_path, fragment_shader_path);
	CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != add_res, "[initial_map]: Failed to add env element.");

	register_drawable_attribute(drawable, POS_TEXTURE_ATTRIBUTE_SIZE); // Position + texutre.
	process_drawable_attributes(drawable);

	Vec3 translation = { { wWidth / 2.f, wHeight / 2.f, Z_DEPTH_INITIAL_MAP_BACKGROUND } };
	drawable->transform.translation = translation;

	Vec3 scale = { { wWidth / 2.f, wHeight / 2.f, 1.f } };
	drawable->transform.scale = scale;

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

	drawable->matrices.projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(drawable->shader_prog, COMMON_PROJECTION_UNIFORM_NAME, &drawable->matrices.projection);

	return 0;
}

int initial_add_path()
{
	// TODO: Replace with quad_draw()?
	static const char* vertex_shader_path = "/res/static/shaders/basic_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/basic_frag.txt";
	static const char* texture_path = "/res/static/textures/road.jpg";

	// TODO: Clear up "magic"
	float path_h = (float)wHeight / 10.f;
	float half_path_h = path_h / 2.f;
	float path_y_offset = (float)wHeight / 6.f;

	PathSegment predefined_path[INTIAL_MAP_PATH_LEN] = {
		{ .start = { 0.f, (float)(wHeight - path_y_offset) }, .end = { 550.f, (float)(wHeight - path_y_offset) } },
		{ .start = { 550.f, (float)(wHeight - path_y_offset) }, .end = { 550.f, (float)path_y_offset } },
		{ .start = { 550.f, (float)path_y_offset }, .end = { 1050.f, (float)path_y_offset } },
		{ .start = { 1050.f, (float)path_y_offset }, .end = { 1050.f, (float)wHeight / 2.f } },
		{ .start = { 1050.f, (float)wHeight / 2.f }, .end = { 1600.f, (float)wHeight / 2.f } }
	};

	PathSegment** path = malloc(INTIAL_MAP_PATH_LEN * sizeof *s_Path);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != path, "[initial_map]: Failed to create path ptr.");

	assert((sizeof(predefined_path) / sizeof(predefined_path[0])) == INTIAL_MAP_PATH_LEN);
	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
	{
		PathSegment* path_seg = malloc(sizeof *path_seg);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != path_seg, "[initial_map]: Failed to create path segment ptr.");

		path[i] = path_seg;
		*path[i] = predefined_path[i];
	}

	s_Path = path;

	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
	{
		DrawBufferData* draw_buf_data = NULL;
		get_quad_draw_buffer_data(&draw_buf_data);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != draw_buf_data, "[initial_map]: Failed to retrieve square DrawBufferData.");

		DrawableDef* drawable = NULL;
		create_drawable(&drawable);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[initial_map]: Drawable is not defined.");

		add_texture_2D(drawable, texture_path, TexType_RGB);

		int add_res = setup_drawable(drawable, draw_buf_data, vertex_shader_path, fragment_shader_path);
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != add_res, "[initial_map]: Failed to add initial map level\'s element.");

		register_drawable_attribute(drawable, POS_TEXTURE_ATTRIBUTE_SIZE); // Position + texture.
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
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != map_func_def, "[initial_map]: Failed to allocate sufficient memory for MapFuncsDef.");

	map_func_def->add_background    = initial_add_background;
	map_func_def->add_path          = initial_add_path;
	map_func_def->free_resources    = initial_free_resources;

	map_func_def->get_path          = get_initial_path;
	map_func_def->get_path_len      = get_initial_path_len;

	map_mgr_register_map(map_func_def);

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
