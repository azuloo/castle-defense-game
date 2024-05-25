#include "graphics.h"
#include "physics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "key_bindings.h"
#include "obj_registry.h"
#include "freetype_text.h"
#include "freetype_renderer.h"
#include "global_defs.h"
#include "drawable_ops.h"
#include "health_bar.h"

#include "map/map_mgr.h"
#include "entity.h"
#include "enemy_wave.h"

#include <string.h>
#include <stdbool.h>

#define DRAW_COLLISION_BOX_BOUNDS 0

int xWOffset   = 0;
int yWOffset   = 0;
int wWidth     = WINDOW_DEFAULT_RES_W;
int wHeight    = WINDOW_DEFAULT_RES_H;

float dt       = 0.0f;  // delta time
float lft      = 0.0f;  // last frame time

static double s_CursorXPos = 0.f;
static double s_CursorYPos = 0.f;

static int s_BuildingModeEnabled = 0;
static int s_CurrentTowerIdx = 0;
static EntityType s_CurrentTowerType = EntityType_None;

static Vec3 s_EnemyScale = { { 35.f, 35.f, 1.f } };

#define TOWER_TYPES_AMOUNT 3
// TODO: Use map here
EntityDef* towers[TOWER_TYPES_AMOUNT];
EntityDef* castle = NULL;

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
		if (NULL == enemy->collidable2D || enemy->collidable2D->handle != collidable->handle)
		{
			continue;
		}

		*dest = enemy;
		break;
	}

	return 0;
}

static int find_tower_with_collidable(EntityDef** dest, const Collidable2D* collidable)
{
	for (int i = 0; i < TOWER_TYPES_AMOUNT; i++)
	{
		if (NULL == towers[i]->collidable2D || towers[i]->collidable2D->handle != collidable->handle)
		{
			continue;
		}

		*dest = towers[i];
		break;
	}

	return 0;
}

static void resolve_entities_collision(EntityDef* first, EntityDef* second)
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

	if (NULL != first_drawable && first->type == EntityType_Castle)
	{
		add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
	}

	if (NULL != second_drawable && second->type == EntityType_Castle)
	{
		add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
	}
}

// TODO: Code duplication (function - process_collision_end_hook())
static void process_collision_begin_hook(Collidable2D* first, Collidable2D* second)
{
	EntityDef* first_entity    = NULL;
	EntityDef* second_entity   = NULL;

	// Enemy - Tower collision.
	if (first->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&first_entity, first);
	}
	else if (first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&first_entity, first);
	}
	else if (first->collision_box.collision_layer & CollisionLayer_Castle)
	{
		first_entity = castle;
	}

	if (second->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&second_entity, second);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&second_entity, second);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Castle)
	{
		second_entity = castle;
	}

	if (NULL != first_entity && NULL != second_entity)
	{
		resolve_entities_collision(first_entity, second_entity);
		return;
	}

	// Tower - Road collision.
	if (NULL != first_entity && first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		if (second->collision_box.collision_layer & CollisionLayer_Road)
		{
			DrawableDef* first_drawable = NULL;
			get_drawable_def(&first_drawable, first_entity->drawable_handle);

			Vec4 color_vec = COLOR_VEC_RED;

			add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
		}
	}

	if (NULL != second_entity && second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		if (first->collision_box.collision_layer & CollisionLayer_Road)
		{
			DrawableDef* second_drawable = NULL;
			get_drawable_def(&second_drawable, second_entity->drawable_handle);

			Vec4 color_vec = COLOR_VEC_RED;

			add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
		}
	}
}

// TODO: Code duplication (function - process_collision_begin_hook())
static void process_collision_end_hook(Collidable2D* first, Collidable2D* second)
{
	EntityDef* first_entity = NULL;
	EntityDef* second_entity = NULL;

	// Enemy - Tower collision.
	if (first->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&first_entity, first);
	}
	else if (first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&first_entity, first);
	}
	else if (first->collision_box.collision_layer & CollisionLayer_Castle)
	{
		first_entity = castle;
	}

	if (second->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&second_entity, second);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&second_entity, second);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Castle)
	{
		second_entity = castle;
	}

	if (NULL != first_entity && NULL != second_entity)
	{
		resolve_entities_collision(first_entity, second_entity);
		return;
	}

	// Tower - Road collision.
	if (NULL != first_entity && first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		if (second->collision_box.collision_layer & CollisionLayer_Road)
		{
			DrawableDef* first_drawable = NULL;
			get_drawable_def(&first_drawable, first_entity->drawable_handle);

			Vec4 color_vec = COLOR_VEC_GREEN;

			add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
		}
	}

	if (NULL != second_entity && second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		if (first->collision_box.collision_layer & CollisionLayer_Road)
		{
			DrawableDef* second_drawable = NULL;
			get_drawable_def(&second_drawable, second_entity->drawable_handle);

			Vec4 color_vec = COLOR_VEC_GREEN;

			add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
		}
	}
}

static void process_key_hook(GWindow* window, int key, int scancode, int action, int mods)
{
	bool key_pressed = false;

	if (key == K_1 && action == KEY_PRESS)
	{
		key_pressed = true;
		s_CurrentTowerIdx = 0;
		s_CurrentTowerType = EntityType_Square;
	}
	if (key == K_2 && action == KEY_PRESS)
	{
		key_pressed = true;
		s_CurrentTowerIdx = 1;
		s_CurrentTowerType = EntityType_Circle;
	}
	if (key == K_3 && action == KEY_PRESS)
	{
		key_pressed = true;
		s_CurrentTowerIdx = 2;
		s_CurrentTowerType = EntityType_Triangle;
	}

	if (key_pressed)
	{
		s_BuildingModeEnabled = 1;
		DrawableDef* tower_drawable = NULL;

		for (int i = 0; i < TOWER_TYPES_AMOUNT; i++)
		{
			get_drawable_def(&tower_drawable, towers[i]->drawable_handle);
			tower_drawable->visible = 0;
		}

		get_drawable_def(&tower_drawable, towers[s_CurrentTowerIdx]->drawable_handle);
		tower_drawable->visible = 1;
	}
}

static void process_mouse_button_hook(GWindow* window, int button, int action, int mods)
{
	if (button == MOUSE_BUTTON_LEFT && action == KEY_PRESS && s_BuildingModeEnabled)
	{
		// Spawn a tower only if it's not colliding with anything.
		if (NULL != towers[s_CurrentTowerIdx]->collidable2D && towers[s_CurrentTowerIdx]->collidable2D->collision_state & CollisionState_Uncollided)
		{
			EntityDef* entity = NULL;

			Vec3 enemy_pos = { { s_CursorXPos, wHeight - s_CursorYPos, Z_DEPTH_INITIAL_ENTITY } };
			Vec4 enemy_color = COLOR_VEC_GREEN;

			switch (s_CurrentTowerType)
			{
			case EntityType_Square:
			{
				add_entity(EntityType_Square, &entity, &enemy_pos, &s_EnemyScale, &enemy_color);
			}
			break;

			case EntityType_Circle:
			{
				add_entity(EntityType_Circle, &entity, &enemy_pos, &s_EnemyScale, &enemy_color);
			}
			break;

			case EntityType_Triangle:
			{
				add_entity(EntityType_Triangle, &entity, &enemy_pos, &s_EnemyScale, &enemy_color);
			}
			break;
			default:
			{
				PRINT_ERR("[game]: Unknown entity type.");
			}
			break;
			}

			if (NULL != entity)
			{
				DrawableDef* drawable = NULL;
				get_drawable_def(&drawable, entity->drawable_handle);
				CHECK_EXPR_FAIL_RET(drawable != NULL, "[game]: Failed to fetch tower drawable.");
				add_collidable2D(&entity->collidable2D, &drawable->transform.translation, &drawable->transform.scale);
				CHECK_EXPR_FAIL_RET(entity->collidable2D != NULL, "[game]: Failed to attach Collidable2D.");
				add_collision_layer2D(&entity->collidable2D->collision_box, CollisionLayer_Tower);
			}

			s_BuildingModeEnabled = !s_BuildingModeEnabled;
		}
	}
}

static int should_be_terminated()
{
	return graphics_should_be_terminated();
}

// TODO: Take the window param into accout
void window_resize_hook(GWindow* window, int x, int y, int width, int height)
{
	xWOffset = x;
	yWOffset = y;
	wWidth = width;
	wHeight = height;
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
	Vec3 tri_scale = { { 35.f, 35.f, 1.f } };
	Vec4 tri_color = COLOR_VEC_GREEN;

	add_entity(EntityType_Triangle, triangle, &tri_pos, &tri_scale, &tri_color);

	return 0;
}

int draw_square_entity(EntityDef** square)
{
	Vec3 sq_pos = { { 900.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 sq_scale = { { 35.f, 35.f, 1.f } };
	Vec4 sq_color = COLOR_VEC_GREEN;

	add_entity(EntityType_Square, square, &sq_pos, &sq_scale, &sq_color);

	return 0;
}

int draw_circle_entity(EntityDef** circle)
{
	Vec3 circle_pos = { { 1100.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 circle_scale = { { 35.f, 35.f, 1.f } };
	Vec4 circle_color = COLOR_VEC_GREEN;

	add_entity(EntityType_Circle, circle, &circle_pos, &circle_scale, &circle_color);

	return 0;
}

int draw_castle_entity(EntityDef** castle)
{
	Vec3 castle_pos = { { 1600.f, wHeight / 2.f, Z_DEPTH_INITIAL_CASTLE } };
	Vec3 castle_scale = { { 125.f, 125.f, 1.f } };
	Vec4 castle_color = COLOR_VEC_WHITE;

	add_entity(EntityType_Castle, castle, &castle_pos, &castle_scale, &castle_color);

	return 0;
}

int create_tower_entities()
{
	// TODO: Add error checks
	EntityDef* tower_entity = NULL;
	DrawableDef* tower_drawable = NULL;

	// TODO: General code; use loop
	draw_square_entity(&tower_entity);
	towers[0] = tower_entity;

	get_drawable_def(&tower_drawable, tower_entity->drawable_handle);
	tower_drawable->visible = 0;

	// TODO: Add error checks
	add_collidable2D(&tower_entity->collidable2D, &tower_drawable->transform.translation, &tower_drawable->transform.scale);
	add_collision_layer2D(&tower_entity->collidable2D->collision_box, CollisionLayer_Tower);
	add_collision_mask2D(&tower_entity->collidable2D->collision_box, CollisionLayer_Road);

	draw_circle_entity(&tower_entity);
	towers[1] = tower_entity;

	get_drawable_def(&tower_drawable, tower_entity->drawable_handle);
	tower_drawable->visible = 0;

	// TODO: Add error checks
	add_collidable2D(&tower_entity->collidable2D, &tower_drawable->transform.translation, &tower_drawable->transform.scale);
	add_collision_layer2D(&tower_entity->collidable2D->collision_box, CollisionLayer_Tower);
	add_collision_mask2D(&tower_entity->collidable2D->collision_box, CollisionLayer_Road);

	draw_triangle_entity(&tower_entity);
	towers[2] = tower_entity;

	get_drawable_def(&tower_drawable, tower_entity->drawable_handle);
	tower_drawable->visible = 0;

	// TODO: Add error checks
	add_collidable2D(&tower_entity->collidable2D, &tower_drawable->transform.translation, &tower_drawable->transform.scale);
	add_collision_layer2D(&tower_entity->collidable2D->collision_box, CollisionLayer_Tower);
	add_collision_mask2D(&tower_entity->collidable2D->collision_box, CollisionLayer_Road);

	return 0;
}

static int add_castle()
{
	draw_castle_entity(&castle);

	DrawableDef* castle_drawable = NULL;
	get_drawable_def(&castle_drawable, castle->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(castle_drawable != NULL, "[game]: Failed to fetch castle drawable.");

	add_collidable2D(&castle->collidable2D, &castle_drawable->transform.translation, &castle_drawable->transform.scale);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != castle->collidable2D, "[game]: Failed to add collidable2D for castle.");
	add_collision_layer2D(&castle->collidable2D->collision_box, CollisionLayer_Castle);
	add_collision_mask2D(&castle->collidable2D->collision_box, CollisionLayer_Enemy);

	const Vec3 pos = { { castle_drawable->transform.translation.x, castle_drawable->transform.translation.y + 150.f, castle_drawable->transform.translation.z } };
	const Vec3 scale = { { 125.f, 12.f, 1.f } };
	add_health_bar(&pos, &scale);

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

	int load_map_res = map_mgr_load_map();
	if (TERMINATE_ERR_CODE == load_map_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	create_tower_entities();
	add_castle();

	init_ft();
	load_ascii_chars();

	ft_renderer_init();
	Vec3 color = { 1.f, 1.f, 1.f };
	render_text("Press 1, 2 or 3 to select Towers", wWidth - 400.f, wHeight - 50.f, color);

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
			CHECK_EXPR_FAIL_RET_TERMINATE(s_CurrentTowerIdx < TOWER_TYPES_AMOUNT, "[game]: currentTowerIdx is greater than towers amount.");

			EntityDef* tower_entity = towers[s_CurrentTowerIdx];
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower_entity, "[game]: Tower entity is empty.");
			DrawableDef* tower_drawable = NULL;

			get_drawable_def(&tower_drawable, tower_entity->drawable_handle);

			if (NULL != tower_drawable)
			{
				float tower_scale_x = tower_drawable->init_transform.scale.x;
				float tower_scale_y = tower_drawable->init_transform.scale.y;
				// TODO: Resize all entities
				resize_entity(tower_entity, tower_scale_x, tower_scale_y);
				move_entity(tower_entity, s_CursorXPos, s_CursorYPos);
				move_collision_box2D(&tower_entity->collidable2D->collision_box, s_CursorXPos, s_CursorYPos);
			}
		}

		physics_step();
		graphics_draw();
	}

	enemy_waves_free_resources();
	map_mgr_free_resources();
	entity_free_resources();
	graphics_free_resources();
	physics_free_resources();
	registry_free();

	return 0;
}
