#include "graphics.h"
#include "physics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "key_bindings.h"
#include "freetype_text.h"
#include "freetype_renderer.h"
#include "global_defs.h"
#include "drawable_ops.h"
#include "health_bar.h"
#include "tower.h"

#include "map/map_mgr.h"
#include "entity.h"
#include "enemy_wave.h"

#include <string.h>
#include <stdbool.h>

int xWOffset           = 0;
int yWOffset           = 0;
int wWidth             = WINDOW_DEFAULT_RES_W;
int wHeight            = WINDOW_DEFAULT_RES_H;
double s_CursorXPos    = 0.f;
double s_CursorYPos    = 0.f;

float dt   = 0.0f;  // Delta time.
float lft  = 0.0f;  // Last frame time.

static int s_BuildingModeEnabled = 0;

extern float get_window_scale_x();
extern float get_window_scale_y();

static int find_enemy_with_collidable(EntityDef** dest, const Collidable2D* collidable)
{
	const EnemyWaveDef* enemy_wave = NULL;
	get_enemy_wave(&enemy_wave);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy_wave, "[game]: Failed to get current enemy wave.");

	EntityDef* enemies = enemy_wave->enemies;
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemies, "[game]: Enemies array is empty.");
	int enemies_amount = enemy_wave->spawned_count;

	EntityDef* enemy = NULL;
	for (int i = 0; i < enemies_amount; i++)
	{
		enemy = enemies + i;
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemies, "[game]: Failed to retrieve an ememy from the enemies array.");

		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, enemy->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[game] Failed to fetch Collidable2D for the enemy.");

		if (enemy->collidable2D_handle != collidable->handle)
		{
			continue;
		}

		*dest = enemy;
		break;
	}

	return 0;
}

static void resolve_entity_castle_collision(EntityDef* first, CastleDef* second)
{
	DrawableDef* first_drawable = NULL;
	get_drawable_def(&first_drawable, first->drawable_handle);

	DrawableDef* second_drawable = NULL;
	get_drawable_def(&second_drawable, second->drawable_handle);

	Vec4 color_vec = COLOR_VEC_RED;

	if (NULL == first_drawable || NULL == second_drawable)
	{
		PRINT_ERR("[game]: Failed to find one or both drawables for provided entities.");
		return;
	}

	Collidable2D* first_collidable2D = NULL;
	get_collidable2D(&first_collidable2D, first->collidable2D_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != first_collidable2D, "[game] Failed to fetch Collidable2D for the entity.");

	if (NULL != first_drawable && first_collidable2D->collision_box.collision_layer & CollisionLayer_Castle)
	{
		map_mgr_damage_castle(10.f);
		add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
	}

	Collidable2D* second_collidable2D = NULL;
	get_collidable2D(&second_collidable2D, second->collidable2D_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != second_collidable2D, "[game] Failed to fetch Collidable2D for the entity.");

	if (NULL != second_drawable && second_collidable2D->collision_box.collision_layer & CollisionLayer_Castle)
	{
		map_mgr_damage_castle(10.f);
		add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
	}
}

// TODO: Code duplication (function - process_collision_end_hook())
static void process_collision_begin_hook(Collidable2D* first, Collidable2D* second)
{
	EntityDef* first_entity = NULL;
	EntityDef* second_entity = NULL;
	CastleDef* castle = NULL;

	if (first->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&first_entity, first);
	}
	else if (first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&first_entity, first);
	}

	if (second->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&second_entity, second);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&second_entity, second);
	}

	if (first->collision_box.collision_layer & CollisionLayer_Castle || second->collision_box.collision_layer & CollisionLayer_Castle)
	{
		map_mgr_get_castle(&castle);
		CHECK_EXPR_FAIL_RET(castle != NULL, "[game]: Failed to get the castle.");
	}

	if (NULL != first_entity)
	{
		if (first->collision_box.collision_layer & ~CollisionLayer_Tower && NULL != castle)
		{
			resolve_entity_castle_collision(first_entity, castle);
			return;
		}

		if (first->collision_box.collision_layer & CollisionLayer_Tower)
		{
			if (second->collision_box.collision_layer & (CollisionLayer_Road | CollisionLayer_Castle))
			{
				DrawableDef* first_drawable = NULL;
				get_drawable_def(&first_drawable, first_entity->drawable_handle);

				Vec4 color_vec = COLOR_VEC_RED;

				add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
	}
	else if (NULL != second_entity)
	{
		if (second->collision_box.collision_layer & ~CollisionLayer_Tower && NULL != castle)
		{
			resolve_entity_castle_collision(second_entity, castle);
			return;
		}

		if (second->collision_box.collision_layer & CollisionLayer_Tower)
		{
			if (first->collision_box.collision_layer & (CollisionLayer_Road | CollisionLayer_Castle))
			{
				DrawableDef* second_drawable = NULL;
				get_drawable_def(&second_drawable, second_entity->drawable_handle);

				Vec4 color_vec = COLOR_VEC_RED;

				add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
	}
}

// TODO: Code duplication (function - process_collision_begin_hook())
static void process_collision_end_hook(Collidable2D* first, Collidable2D* second)
{
	EntityDef* first_entity = NULL;
	EntityDef* second_entity = NULL;
	CastleDef* castle = NULL;

	if (first->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&first_entity, first);
	}
	else if (first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&first_entity, first);
	}

	if (second->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&second_entity, second);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&second_entity, second);
	}

	if (first->collision_box.collision_layer & CollisionLayer_Castle || second->collision_box.collision_layer & CollisionLayer_Castle)
	{
		map_mgr_get_castle(&castle);
		CHECK_EXPR_FAIL_RET(castle != NULL, "[game]: Failed to get the castle.");
	}

	if (NULL != first_entity)
	{
		Collidable2D* first_collidable2D = NULL;
		get_collidable2D(&first_collidable2D, first_entity->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != first_collidable2D, "[game] Failed to fetch Collidable2D for the entity.");

		if (NULL != castle && first_collidable2D->collision_box.collision_layer & ~CollisionLayer_Tower)
		{
			resolve_entity_castle_collision(first_entity, castle);
			return;
		}

		if (first->collision_box.collision_layer & CollisionLayer_Tower)
		{
			bool road_collision_pred = second->collision_box.collision_layer & CollisionLayer_Road && first_collidable2D->collisions_detected == 0;
			bool castle_collision_pred = second->collision_box.collision_layer & CollisionLayer_Castle && first_collidable2D->collisions_detected == 0;
			if (road_collision_pred || castle_collision_pred)
			{
				DrawableDef* first_drawable = NULL;
				get_drawable_def(&first_drawable, first_entity->drawable_handle);

				Vec4 color_vec = COLOR_VEC_GREEN;

				add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
	}
	if (NULL != second_entity)
	{
		Collidable2D* second_collidable2D = NULL;
		get_collidable2D(&second_collidable2D, second_entity->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != second_collidable2D, "[game] Failed to fetch Collidable2D for the entity.");

		if (NULL != castle && second_collidable2D->collision_box.collision_layer & ~CollisionLayer_Tower)
		{
			resolve_entity_castle_collision(second_entity, castle);
			return;
		}

		if (second->collision_box.collision_layer & CollisionLayer_Tower)
		{
			bool road_collision_pred = first->collision_box.collision_layer & CollisionLayer_Road && second_collidable2D->collisions_detected == 0;
			bool castle_collision_pred = first->collision_box.collision_layer & CollisionLayer_Castle && second_collidable2D->collisions_detected == 0;
			if (road_collision_pred || castle_collision_pred)
			{
				DrawableDef* second_drawable = NULL;
				get_drawable_def(&second_drawable, second_entity->drawable_handle);

			    Vec4 color_vec = COLOR_VEC_GREEN;

				add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
	}
}

static void process_key_hook(GWindow* window, int key, int scancode, int action, int mods)
{
	bool key_pressed = false;

	if (key == K_1 && action == KEY_PRESS)
	{
		key_pressed = true;
		set_current_tower_preset_idx(0);
	}
	if (key == K_2 && action == KEY_PRESS)
	{
		key_pressed = true;
		set_current_tower_preset_idx(1);
	}
	if (key == K_3 && action == KEY_PRESS)
	{
		key_pressed = true;
		set_current_tower_preset_idx(2);
	}

	if (key_pressed)
	{
		s_BuildingModeEnabled = 1;
		on_select_tower_preset_pressed();
	}
}

static void process_mouse_button_hook(GWindow* window, int button, int action, int mods)
{
	if (button == MOUSE_BUTTON_LEFT && action == KEY_PRESS && s_BuildingModeEnabled)
	{
		if (place_new_tower_at_cursor())
		{
			s_BuildingModeEnabled = !s_BuildingModeEnabled;
		}
	}
}

static int should_be_terminated()
{
	return graphics_should_be_terminated();
}

float get_window_scale_x()
{
	return (float)wWidth / WINDOW_DEFAULT_RES_W;
}

float get_window_scale_y()
{
	return (float)wHeight / WINDOW_DEFAULT_RES_H;
}

// TODO: Take the window param into accout
void window_resize_hook(GWindow* window, int x, int y, int width, int height)
{
	xWOffset = x;
	yWOffset = y;
	wWidth = width;
	wHeight = height;

	map_mgr_on_window_resize();
	enemy_wave_on_window_resize();
	resize_towers();
}

void process_input(GWindow* window)
{
	if (KEY_PRESSED(window, K_ESCAPE))
	{
		close_window(window);
	}
}

int draw_triangle_entity(EntityDef** triangle)
{
	Vec3 tri_pos = { { 700.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 tri_scale = { { (float)wHeight * 0.03f, (float)wHeight * 0.03f, 1.f } };
	Vec4 tri_color = COLOR_VEC_GREEN;

	add_entity(EntityType_Triangle, triangle, &tri_pos, &tri_scale, &tri_color);

	return 0;
}

int draw_square_entity(EntityDef** square)
{
	Vec3 sq_pos = { { 900.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 sq_scale = { { (float)wHeight * 0.03f, (float)wHeight * 0.03f, 1.f } };
	Vec4 sq_color = COLOR_VEC_GREEN;

	add_entity(EntityType_Square, square, &sq_pos, &sq_scale, &sq_color);

	return 0;
}

int draw_circle_entity(EntityDef** circle)
{
	Vec3 circle_pos = { { 1100.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 circle_scale = { { (float)wHeight * 0.03f, (float)wHeight * 0.03f, 1.f } };
	Vec4 circle_color = COLOR_VEC_GREEN;

	add_entity(EntityType_Circle, circle, &circle_pos, &circle_scale, &circle_color);

	return 0;
}

int main(int argc, int* argv[])
{
	int init_graphics_res = init_graphics();
	if (TERMINATE_ERR_CODE == init_graphics_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	bind_input_fn(&process_input);
	bind_window_resize_fn(&window_resize_hook);
	bind_key_pressed_cb(&process_key_hook);
	bind_mouse_button_cb(&process_mouse_button_hook);
	physics_bind_collision_begind_cb(&process_collision_begin_hook);
	physics_bind_collision_end_cb(&process_collision_end_hook);

	map_mgr_init();
	map_mgr_add_castle();

	int load_map_res = map_mgr_load_map();
	if (TERMINATE_ERR_CODE == load_map_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	create_build_tower_presets();

	init_ft();
	load_ascii_chars();

	ft_renderer_init();
	Vec3 color = { 1.f, 1.f, 1.f };
	render_text("Press 1, 2 or 3 to select Towers", wWidth - 400.f, wHeight - 50.f, color);

	float tower_x_pos = 0.f;
	float tower_y_pos = 0.f;

	// TODO: Handle Windows window drag (other events?)
	while (!should_be_terminated())
	{
		float curr_time = (float)glfwGetTime();
		dt = curr_time - lft;
		lft = curr_time;

		dt = math_clamp(dt, 0.f, MAX_FRAME_TIME);

		enemy_waves_spawn(dt);

		if (s_BuildingModeEnabled)
		{
			graphics_get_cursor_pos(&s_CursorXPos, &s_CursorYPos);
			on_tower_building_mode_enabled();
		}

		physics_step();
		graphics_draw();
	}

	enemy_waves_free_resources();
	map_mgr_free_resources();
	entity_free_resources();
	graphics_free_resources();
	physics_free_resources();

	return 0;
}
