#include "map/initial_map.h"
#include "map/map_mgr.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "global_defs.h"
#include "drawable_ops.h"
#include "physics.h"
#include "enemy_wave.h"
#include "health_bar.h"

#define INTIAL_MAP_PATH_LEN 5
#define INITIAL_ENEMY_WAVES 5
#define PATH_HEIGHT 0.05f

extern int wWidth;
extern int wHeight;

extern float get_window_scale_x();
extern float get_window_scale_y();

static const char* road_texture_path = "/res/static/textures/road.jpg";
static const char* field_texture_path = "/res/static/textures/field.jpg";
static const char* castle_texture_path = "/res/static/textures/castle.png";

static PathSegment s_PathSegments[INTIAL_MAP_PATH_LEN];
static PathDef s_PathDef[INTIAL_MAP_PATH_LEN];

CastleDef* s_Castle = NULL;

static const EnemyWaveCnf s_EnemyWaveCnf[INITIAL_ENEMY_WAVES] = {
	{ EnemyWaveType_Random, 5, 3.f, 1.f },
	{ EnemyWaveType_Random, 6, 3.f, 1.f },
	{ EnemyWaveType_Random, 7, 3.f, 1.f },
	{ EnemyWaveType_Random, 5, 3.f, 1.f },
	{ EnemyWaveType_Random, 5, 3.f, 1.f },
};

static int map_init()
{
	int res = init_enemy_waves(INITIAL_ENEMY_WAVES);
	CHECK_EXPR_FAIL_RET_TERMINATE(res != TERMINATE_ERR_CODE, "[initial_map]: Failed to init emeny waves.");

	set_enemy_waves_cnf(s_EnemyWaveCnf, INITIAL_ENEMY_WAVES);

	return 0;
}

static int add_background()
{
	Vec3 translation = { { wWidth / 2.f, wHeight / 2.f, Z_DEPTH_INITIAL_MAP_BACKGROUND } };
	Vec3 scale = { { wWidth / 2.f, wHeight / 2.f, 1.f } };
	Vec4 color = COLOR_VEC_WHITE;

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, field_texture_path, TexType_RGB, &translation, &scale, &color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[initial_map]: Failed to draw triangle entity (empty quad drawable).");

	return 0;
}

static int init_path_segments()
{
	PathSegment predefined_path[INTIAL_MAP_PATH_LEN] = {
		{ .start = { 0.f, (float)wHeight * 0.8f }, .end = { (float)wWidth * 0.2f, (float)wHeight * 0.8f } },
		{ .start = { (float)wWidth * 0.2f, (float)wHeight * 0.8f }, .end = { (float)wWidth * 0.2f, (float)wHeight * 0.2f } },
		{ .start = { (float)wWidth * 0.2f, (float)wHeight * 0.2f }, .end = { (float)wWidth * 0.5f, (float)wHeight * 0.2f } },
		{ .start = { (float)wWidth * 0.5f, (float)wHeight * 0.2f }, .end = { (float)wWidth * 0.5f, (float)wHeight * 0.5f } },
		{ .start = { (float)wWidth * 0.5f, (float)wHeight * 0.5f  }, .end = { (float)wWidth * 0.8f, (float)wHeight * 0.5f } }
	};

	memcpy(s_PathSegments, predefined_path, INTIAL_MAP_PATH_LEN * sizeof(PathSegment));
	CHECK_EXPR_FAIL_RET_TERMINATE((sizeof(predefined_path) / sizeof(predefined_path[0])) == INTIAL_MAP_PATH_LEN, "[inital_map]: Wrong predefined path length.");
	
	return 0;
}

static int calculate_segment_translation(const PathSegment* path_segment, Vec3* translation)
{
	float pos_x = (path_segment->start.x + path_segment->end.x) / 2.f;
	float pos_y = (path_segment->start.y + path_segment->end.y) / 2.f;

	Vec3 new_translation = { { pos_x, pos_y, Z_DEPTH_INITIAL_MAP_PATH } };
	*translation = new_translation;

	return 0;
}

static int calculate_segment_scale(const PathSegment* path_segment, Vec3* scale)
{
	float pos_x_diff = path_segment->end.x - path_segment->start.x;
	float pos_y_diff = path_segment->end.y - path_segment->start.y;

	float scale_x = 0;
	float scale_y = 0;
	float path_h = (float)wHeight * PATH_HEIGHT;

	if (pos_x_diff == 0)
	{
		if (pos_y_diff < 0)
		{
			scale_y = pos_y_diff / 2.f - path_h;
		}
		else
		{
			scale_y = pos_y_diff / 2.f + path_h;
		}

		scale_x = path_h;
	}
	else if (pos_y_diff == 0)
	{
		if (pos_x_diff < 0)
		{
			scale_x = pos_x_diff / 2.f - path_h;
		}
		else
		{
			scale_x = pos_x_diff / 2.f + path_h;
		}

		scale_y = path_h;
	}

	float new_scale_x = fabsf(scale_x);
	float new_scale_y = fabsf(scale_y);

	Vec3 new_scale = { { new_scale_x, new_scale_y, 1.f } };
	*scale = new_scale;

	return 0;
}

static int recalculate_path()
{
	init_path_segments();

	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
	{
		s_PathDef[i].path_segment = s_PathSegments[i];
		const PathSegment* path_segment = &s_PathDef[i].path_segment;

		Vec3 translation;
		calculate_segment_translation(path_segment, &translation);

		Vec3 scale;
		calculate_segment_scale(path_segment, &scale);

		DrawableDef* drawable = NULL;
		get_drawable_def(&drawable, s_PathDef[i].drawable_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[initial_map] Failed to fetch drawable.");

		drawable->transform.translation = translation;
		drawable->transform.scale = scale;

		if (NULL != s_PathDef[i].collidable2D)
		{
			move_collision_box2D(&s_PathDef[i].collidable2D->collision_box, translation.x, translation.y);
			resize_collision_box2D(&s_PathDef[i].collidable2D->collision_box, scale.x, scale.y);
		}

		drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);
	}
}

static int add_path()
{
	Vec4 color = COLOR_VEC_WHITE;

	init_path_segments();

	for (int i = 0; i < INTIAL_MAP_PATH_LEN; i++)
	{
		s_PathDef[i].path_segment = s_PathSegments[i];
		s_PathDef[i].collidable2D = NULL;

		const PathSegment* path_segment = &s_PathDef[i].path_segment;
		
		Vec3 translation;
		calculate_segment_translation(path_segment, &translation);

		Vec3 scale;
		calculate_segment_scale(path_segment, &scale);

		DrawableDef* drawable = NULL;
		draw_quad(&drawable, road_texture_path, TexType_RGB, &translation, &scale, &color);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[initial_map]: Failed to draw triangle entity (empty quad drawable).");

		s_PathDef[i].drawable_handle = drawable->handle;

		// TODO: Add NULL check
		add_collidable2D(&s_PathDef[i].collidable2D, &drawable->transform.translation, &drawable->transform.scale);
		add_collision_layer2D(&s_PathDef[i].collidable2D->collision_box, CollisionLayer_Road);
	}

	return 0;
}

static Vec2 get_init_direction()
{
	return Vec2_RIGHT;
}

static int create_castle()
{
	if (NULL != s_Castle)
	{
		return;
	}

	CastleDef* castle = malloc(sizeof * castle);
	CHECK_EXPR_FAIL_RET_TERMINATE(castle != NULL, "[game]: Failed to create the castle.");

	castle->drawable_handle = -1;
	// TODO: Entity should be generalized or we should use different type for castle
	castle->type = EntityType_Castle;
	castle->collidable2D = NULL;

	s_Castle = castle;

	return 0;
}

static int resize_castle()
{
	if (NULL == s_Castle)
	{
		return TERMINATE_ERR_CODE;
	}

	Vec3 castle_pos = { { (float)wWidth * 0.8f, (float)wHeight * 0.5f, Z_DEPTH_INITIAL_CASTLE } };
	Vec3 castle_scale = { { (float)wHeight * 0.11f, (float)wHeight * 0.11f, 1.f } };

	DrawableDef* castle_drawable = NULL;
	get_drawable_def(&castle_drawable, s_Castle->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(castle_drawable != NULL, "[game]: Failed to get the castle drawable.");

	castle_drawable->transform.translation = castle_pos;
	castle_drawable->transform.scale = castle_scale;

	if (NULL != s_Castle->collidable2D)
	{
		move_collision_box2D(&s_Castle->collidable2D->collision_box, castle_pos.x, castle_pos.y);
		resize_collision_box2D(&s_Castle->collidable2D->collision_box, castle_scale.x, castle_scale.y);
	}

	// TODO: Add resizing castle health bar once we add it's state to the Castle

	drawable_transform_ts(castle_drawable, COMMON_MODEL_UNIFORM_NAME);

	return 0;
}

static int add_castle()
{
	if (NULL == s_Castle)
	{
		int create_castle_res = create_castle();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_SUCCESS_CODE == create_castle_res, "[initial_map]: Castle creation function failed.");
	}

	Vec3 castle_pos = { { (float)wWidth * 0.8f, (float)wHeight * 0.5f, Z_DEPTH_INITIAL_CASTLE } };
	Vec3 castle_scale = { { (float)wHeight * 0.11f, (float)wHeight * 0.11f, 1.f } };
	Vec4 castle_color = COLOR_VEC_WHITE;

	DrawableDef* castle_drawable = NULL;
	draw_quad(&castle_drawable, castle_texture_path, TexType_RGBA, &castle_pos, &castle_scale, &castle_color);
	CHECK_EXPR_FAIL_RET_TERMINATE(castle_drawable != NULL, "[game]: Failed to draw the castle.");

	s_Castle->drawable_handle = castle_drawable->handle;

	add_collidable2D(&s_Castle->collidable2D, &castle_drawable->transform.translation, &castle_drawable->transform.scale);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_Castle->collidable2D, "[game]: Failed to add collidable2D for the castle.");
	add_collision_layer2D(&s_Castle->collidable2D->collision_box, CollisionLayer_Castle);
	add_collision_mask2D(&s_Castle->collidable2D->collision_box, CollisionLayer_Enemy);

	const Vec3 hb_pos = { { castle_drawable->transform.translation.x, castle_drawable->transform.translation.y + (float)wHeight * 0.13f, castle_drawable->transform.translation.z } };
	const Vec3 hb_scale = { { (float)wHeight * 0.11f, (float)wHeight * 0.01f, 1.f } };
	add_health_bar(&hb_pos, &hb_scale);

	return 0;
}

static int on_window_resize()
{
	recalculate_path();
	resize_castle();
}

static Vec2 get_path_start()
{
	return s_PathDef[0].path_segment.start;
}

static void free_map_resources()
{
	if (NULL != s_Castle)
	{
		free(s_Castle);
	}
}

static const PathDef* get_path()
{
	return s_PathDef;
}

static int get_path_len()
{
	return INTIAL_MAP_PATH_LEN;
}

int initial_map_init()
{
	MapFuncsDef* map_func_def = malloc(sizeof * map_func_def);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != map_func_def, "[initial_map]: Failed to allocate sufficient memory for MapFuncsDef.");

	map_func_def->map_init             = map_init;
	map_func_def->add_background       = add_background;
	map_func_def->add_path             = add_path;
	map_func_def->get_init_direction   = get_init_direction;
	map_func_def->add_castle           = add_castle;
	map_func_def->on_window_resize     = on_window_resize;
	map_func_def->get_path_start       = get_path_start;
	map_func_def->free_resources       = free_map_resources;

	map_func_def->get_path             = get_path;
	map_func_def->get_path_len         = get_path_len;

	map_mgr_register_map(map_func_def);

	return 0;
}
