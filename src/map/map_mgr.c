#include "map/map_mgr.h"
#include "map/initial_map.h"
#include "physics.h"
#include "graphics.h"
#include "utils.h"
#include "enemy_wave.h"
#include "global_defs.h"
#include "drawable_ops.h"
#include "health_bar.h"
#include "hud.h"
#include "game_control.h"

extern int wWidth;
extern int wHeight;

static int              s_CurrMapIdx = 0;
static int              s_NextAvailableIdx = 0;
static int              s_MapFunctionsCapacity = 32;
static MapFuncsDef**    s_MapFunctions;

static CastleDef* s_Castle = NULL;

static const char* castle_texture_path = "/res/static/textures/castle.png";

// ! Allocates memory on heap !
static int init_map_functions()
{
	s_MapFunctionsCapacity *= 2;
	MapFuncsDef** map_funcs_ptr = realloc(s_MapFunctions, s_MapFunctionsCapacity * sizeof *map_funcs_ptr);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != map_funcs_ptr, "[map_mgr]: Failed to allocate sufficient memory for MapFuncsDef arr.");

	s_MapFunctions = map_funcs_ptr;
	
	return 0;
}

static const MapFuncsDef* get_curr_map()
{
	MapFuncsDef* map_def = s_MapFunctions[s_CurrMapIdx];
	CHECK_EXPR_FAIL_RET_NULL(NULL != map_def, "[map_mgr]: Failed to load current map funcs def.");

	return map_def;
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
	castle->collidable2D_handle = -1;

	s_Castle = castle;

	return 0;
}

static int resize_castle()
{
	if (NULL == s_Castle)
	{
		return TERMINATE_ERR_CODE;
	}

	Vec3 castle_pos = { { (float)wWidth * 0.8f, (float)wHeight * 0.5f, 0.f } };
	Vec3 castle_scale = { { (float)wHeight * 0.11f, (float)wHeight * 0.11f, 1.f } };

	DrawableDef* castle_drawable = NULL;
	get_drawable_def(&castle_drawable, s_Castle->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(castle_drawable != NULL, "[game]: Failed to get the castle drawable.");

	castle_drawable->transform.translation = castle_pos;
	castle_drawable->transform.scale = castle_scale;

	if (-1 != s_Castle->collidable2D_handle)
	{
		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, s_Castle->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[initial_map] Failed to fetch Collidable2D for the castle.");

		move_collision_box2D(&collidable2D->collision_box, castle_pos.x, castle_pos.y);
		resize_collision_box2D(&collidable2D->collision_box, castle_scale.x, castle_scale.y);
	}

	// TODO: Add resizing castle health bar once we add it's state to the Castle

	drawable_transform_ts(castle_drawable, COMMON_MODEL_UNIFORM_NAME);

	return 0;
}

int map_mgr_init()
{
	if (NULL == s_MapFunctions)
	{
		int map_arr_alloc_res = init_map_functions();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != map_arr_alloc_res, "[map_mgr]: Failed to register a new map (map func arr allocation failed).");
	}

	initial_map_init();

	return 0;
}

int map_mgr_register_map(const MapFuncsDef* map_funcs_def)
{
	if (s_NextAvailableIdx >= s_MapFunctionsCapacity)
	{
		int map_arr_alloc_res = init_map_functions();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != map_arr_alloc_res, "[map_mgr]: Failed to register a new map (map func arr allocation failed).");
	}

	s_MapFunctions[s_NextAvailableIdx] = map_funcs_def;
	s_NextAvailableIdx++;

	return 0;
}

int map_mgr_load_map()
{
	const MapFuncsDef* map_def = get_curr_map();
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != map_def, "[map_mgr]: Failed to get current map ptr.");

	// TODO: Add NULL ptr checks
	map_def->map_init();
	map_def->add_background();
	map_def->add_path();

	return 0;
}

int map_mgr_advance_to_next_map()
{
	// TODO: Do we need to clear prev map resources?
	s_CurrMapIdx++;
	// TODO: If we've reached the end - return some op code
	return 0;
}

int map_mgr_on_window_resize()
{
	resize_castle();

	const MapFuncsDef* map_def = get_curr_map();
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != map_def, "[map_mgr]: Failed to get current map ptr.");

	if (NULL != map_def->on_window_resize)
	{
		map_def->on_window_resize();
	}

	return 0;
}

Vec2 map_mgr_get_init_direction()
{
	Vec2 init_direction = { 0.f, 0.f };
	const MapFuncsDef* map_def = get_curr_map();

	if (NULL != map_def)
	{
		init_direction = map_def->get_init_direction();
	}

	return init_direction;
}

const PathDef* map_mgr_get_path()
{
	const MapFuncsDef* map_def = get_curr_map();
	CHECK_EXPR_FAIL_RET_NULL(NULL != map_def, "[map_mgr]: Failed to get current map ptr.");

	return map_def->get_path();
}

Vec2 map_mgr_get_path_start()
{
	Vec2 path_start = { 0.f, 0.f };
	const MapFuncsDef* map_def = get_curr_map();

	if (NULL != map_def)
	{
		path_start = map_def->get_path_start();
	}

	return path_start;
}

int map_mgr_get_path_len()
{
	const MapFuncsDef* map_def = get_curr_map();
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != map_def, "[map_mgr]: Failed to get current map ptr.");

	return map_def->get_path_len();
}

int map_mgr_add_castle()
{
	if (NULL == s_Castle)
	{
		int create_castle_res = create_castle();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_SUCCESS_CODE == create_castle_res, "[initial_map]: Castle creation function failed.");
	}

	Vec3 castle_pos = { { (float)wWidth * 0.8f, (float)wHeight * 0.5f, 0.f } };
	Vec3 castle_scale = { { (float)wHeight * 0.11f, (float)wHeight * 0.11f, 1.f } };
	Vec4 castle_color = COLOR_VEC_WHITE;

	DrawableDef* castle_drawable = NULL;
	draw_quad(&castle_drawable, &castle_pos, &castle_scale, &castle_color, DrawLayer_Castle);
	CHECK_EXPR_FAIL_RET_TERMINATE(castle_drawable != NULL, "[initial_map]: Failed to draw the castle.");
	add_texture_2D(castle_drawable, castle_texture_path, TexType_RGBA, default_texture_params, DEFAULT_TEXTURE_PARAMS_COUNT);

	s_Castle->drawable_handle = castle_drawable->handle;

	add_collidable2D(&s_Castle->collidable2D_handle, &castle_drawable->transform.translation, &castle_drawable->transform.scale);
	Collidable2D* collidable2D = NULL;
	get_collidable2D(&collidable2D, s_Castle->collidable2D_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[initial_map] Failed to fetch Collidable2D for the castle.");

	add_collision_layer2D(&collidable2D->collision_box, CollisionLayer_Castle);
	add_collision_mask2D(&collidable2D->collision_box, CollisionLayer_Enemy);

	const Vec3 hb_pos = { { castle_drawable->transform.translation.x, castle_drawable->transform.translation.y + (float)wHeight * 0.13f, castle_drawable->transform.translation.z } };
	const Vec3 hb_scale = { { (float)wHeight * 0.11f, (float)wHeight * 0.01f, 1.f } };

	int health_bar_handle = -1;
	add_health_bar(&health_bar_handle, &hb_pos, &hb_scale);
	CHECK_EXPR_FAIL_RET_TERMINATE(-1 != health_bar_handle, "[initial_map] Failed to fetch a health bar for the castle.");

	s_Castle->health_bar_handle = health_bar_handle;

	return 0;
}

int map_mgr_get_castle(CastleDef** dest)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_Castle, "[map_mgr]: The castle has not been initialized.");
	*dest = s_Castle;

	return 0;
}

int map_mgr_damage_castle(float amount)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_Castle, "[map_mgr]: The castle has not been initialized.");
	change_health_bar_value(s_Castle->health_bar_handle, amount);

	// TODO: Until we have an event system, it would be here
	float castle_health_left = 0.f;
	get_health_bar_value(s_Castle->health_bar_handle, &castle_health_left);

	if (castle_health_left <= 0.f)
	{
		render_game_over();
		stop_game();
	}

	return 0;
}

int map_mgr_free_resources()
{
	if (NULL != s_Castle)
	{
		free(s_Castle);
	}

	if (NULL == s_MapFunctions)
	{
		return 0;
	}

	for (int i = 0; i < s_NextAvailableIdx; i++)
	{
		if (NULL != s_MapFunctions[i]->free_resources)
		{
			s_MapFunctions[i]->free_resources();
		}
		
		free(s_MapFunctions[i]);
	}

	free(s_MapFunctions);

	return 0;
}
