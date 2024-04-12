#include "map/initial_map.h"
#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "map/map_mgr.h"
#include "global_defs.h"
#include "graphics_defs.h"
#include "drawable_ops.h"
#include "physics.h"
#include "obj_registry.h"

#define INTIAL_MAP_PATH_LEN 5

extern int wWidth;
extern int wHeight;

static PathDef s_PathDef[INTIAL_MAP_PATH_LEN];

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int initial_add_background()
{
	static const char* texture_path = "/res/static/textures/field.jpg";

	Vec3 translation = { { wWidth / 2.f, wHeight / 2.f, Z_DEPTH_INITIAL_MAP_BACKGROUND } };
	Vec3 scale = { { wWidth / 2.f, wHeight / 2.f, 1.f } };
	Vec4 color = { { 1.f, 1.f, 1.f, 1.f } };

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, texture_path, TexType_RGB, &translation, &scale, &color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw triangle entity (empty quad drawable).");

	return 0;
}

int initial_add_path()
{
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

	assert((sizeof(predefined_path) / sizeof(predefined_path[0])) == INTIAL_MAP_PATH_LEN);

	Vec4 color = { { 1.f, 1.f, 1.f, 1.f } };

	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
	{
		s_PathDef[i].path_segment    = predefined_path[i];
		s_PathDef[i].collidable2D    = NULL;

		PathSegment* path_segment    = &s_PathDef[i].path_segment;

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
		Vec3 scale = { { fabsf(scale_x), fabsf(scale_y), 1.f } };

		DrawableDef* drawable = NULL;
		draw_quad(&drawable, texture_path, TexType_RGB, &translation, &scale, &color);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw triangle entity (empty quad drawable).");

		s_PathDef[i].drawable_handle = drawable->handle;

		// TODO: Add NULL check
		add_collidable2D(&s_PathDef[i].collidable2D, &drawable->transform.translation, &drawable->transform.scale);
		add_collision_layer2D(s_PathDef[i].collidable2D->collision_box, CollisionLayer_Road);
	}

	return 0;
}

void initial_free_resources()
{
	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
	{
		if (NULL != s_PathDef[i].collidable2D)
		{
			if (NULL != s_PathDef[i].collidable2D->collision_box)
			{
				free(s_PathDef[i].collidable2D->collision_box);
			}

			free(s_PathDef[i].collidable2D);
		}
	}
}

const PathDef* get_initial_path()
{
	return s_PathDef;
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
