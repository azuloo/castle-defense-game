#include "graphics.h"
#include "physics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "key_bindings.h"
#include "obj_registry.h"
#include "graphics_defs.h"
#include "freetype_text.h"
#include "freetype_renderer.h"
#include "global_defs.h"
#include "drawable_ops.h"

#include "map/map_mgr.h"
#include "entity.h"

#include <string.h>
#include <stdbool.h>

int wWidth     = WINDOW_DEFAULT_RES_W;
int wHeight    = WINDOW_DEFAULT_RES_H;

float dt       = 0.0f;  // delta time
float lft      = 0.0f;  // last frame time

double cursor_xpos = 0.f;
double cursor_ypos = 0.f;

int s_buildingModeEnabled = 0;
int s_currentTowerIdx = 0;
EntityType s_currentTowerType = Entity_None;

#define TOWERS_AMOUNT 3
#define ENEMIES_AMOUNT 3
// TODO: Use map here
EntityDef* towers[TOWERS_AMOUNT];
EntityDef* enemies[ENEMIES_AMOUNT];
EntityDef* castle = NULL;

// TODO: Need HEAVY optimization
static int find_enemy_with_collidable(EntityDef** dest, const Collidable2D* collidable)
{
	for (int i = 0; i < ENEMIES_AMOUNT; i++)
	{
		if (NULL == enemies[i]->collidable2D || NULL == enemies[i]->collidable2D->collision_box || enemies[i]->collidable2D->handle != collidable->handle)
		{
			continue;
		}

		*dest = enemies[i];
		break;
	}

	return 0;
}

// TODO: Need HEAVY optimization
static int find_tower_with_collidable(EntityDef** dest, const Collidable2D* collidable)
{
	for (int i = 0; i < ENEMIES_AMOUNT; i++)
	{
		if (NULL == towers[i]->collidable2D || NULL == towers[i]->collidable2D->collision_box || towers[i]->collidable2D->handle != collidable->handle)
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

	if (NULL != first_drawable && first->type == Entity_Castle)
	{
		add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
	}

	if (NULL != second_drawable && second->type == Entity_Castle)
	{
		add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
	}
}

static void resolve_entity_road_collision(EntityDef* first, EntityDef* second)
{
	DrawableDef* first_drawable = NULL;
	get_drawable_def(&first_drawable, first);

	DrawableDef* second_drawable = NULL;
	get_drawable_def(&second_drawable, second->drawable_handle);


}

static void process_collision_event_hook(Collidable2D* first, Collidable2D* second)
{
	EntityDef* first_entity    = NULL;
	EntityDef* second_entity   = NULL;

	if (first->collision_box->collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&first_entity, first);
	}
	else if (first->collision_box->collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&first_entity, first);
	}
	else if (first->collision_box->collision_layer & CollisionLayer_Castle)
	{
		first_entity = castle;
	}

	if (second->collision_box->collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&second_entity, second);
	}
	else if (second->collision_box->collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&second_entity, second);
	}
	else if (second->collision_box->collision_layer & CollisionLayer_Castle)
	{
		second_entity = castle;
	}

	if (NULL != first_entity && NULL != second_entity)
	{
		resolve_entities_collision(first_entity, second_entity);
		return;
	}

	// Tower - Road collision
	if (NULL != first_entity && first->collision_box->collision_layer & CollisionLayer_Tower)
	{
		if (second->collision_box->collision_layer & CollisionLayer_Road)
		{
			DrawableDef* first_drawable = NULL;
			get_drawable_def(&first_drawable, first_entity->drawable_handle);

			Vec4 color_vec = COLOR_VEC_GREEN;

			if (first->collision_state & CollisionState_Collided)
			{
				color_vec = COLOR_VEC_RED;
			}

			add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
		}
	}

	if (NULL != second_entity && second->collision_box->collision_layer & CollisionLayer_Tower)
	{
		if (first->collision_box->collision_layer & CollisionLayer_Road)
		{
			DrawableDef* second_drawable = NULL;
			get_drawable_def(&second_drawable, second_entity->drawable_handle);

			Vec4 color_vec = COLOR_VEC_GREEN;

			if (second->collision_state & CollisionState_Collided)
			{
				color_vec = COLOR_VEC_RED;
			}

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
		s_currentTowerIdx = 0;
		s_currentTowerType = Entity_Square;
	}
	if (key == K_2 && action == KEY_PRESS)
	{
		key_pressed = true;
		s_currentTowerIdx = 1;
		s_currentTowerType = Entity_Circle;
	}
	if (key == K_3 && action == KEY_PRESS)
	{
		key_pressed = true;
		s_currentTowerIdx = 2;
		s_currentTowerType = Entity_Triangle;
	}

	if (key_pressed)
	{
		s_buildingModeEnabled = 1;
		DrawableDef* tower_drawable = NULL;

		for (int i = 0; i < TOWERS_AMOUNT; i++)
		{
			get_drawable_def(&tower_drawable, towers[i]->drawable_handle);
			tower_drawable->visible = 0;
		}

		get_drawable_def(&tower_drawable, towers[s_currentTowerIdx]->drawable_handle);
		tower_drawable->visible = 1;
	}
}

static void process_mouse_button_hook(GWindow* window, int button, int action, int mods)
{
	if (button == MOUSE_BUTTON_LEFT && action == KEY_PRESS && s_buildingModeEnabled)
	{
		// Spawn a tower only if it's not colliding with anything.
		if (NULL != towers[s_currentTowerIdx]->collidable2D && towers[s_currentTowerIdx]->collidable2D->collision_state & CollisionState_Uncollided)
		{
			EntityDef* entity = NULL;

			// TODO: Common data; move
			Vec3 sq_pos = { { cursor_xpos, wHeight - cursor_ypos, Z_DEPTH_INITIAL_ENTITY } };
			Vec3 sq_scale = { { 35.f, 35.f, 1.f } };
			Vec4 sq_color = COLOR_VEC_GREEN;

			switch (s_currentTowerType)
			{
			case Entity_Square:
			{
				add_entity(Entity_Square, &entity, &sq_pos, &sq_scale, &sq_color);
			}
			break;

			case Entity_Circle:
			{
				add_entity(Entity_Circle, &entity, &sq_pos, &sq_scale, &sq_color);
			}
			break;

			case Entity_Triangle:
			{
				add_entity(Entity_Triangle, &entity, &sq_pos, &sq_scale, &sq_color);
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
				CHECK_EXPR_FAIL_RET(entity->collidable2D != NULL && entity->collidable2D->collision_box, "[game]: Failed to attach Collidable2D.");
				add_collision_layer2D(entity->collidable2D->collision_box, CollisionLayer_Tower);
			}

			s_buildingModeEnabled = !s_buildingModeEnabled;
		}
	}
}

static int should_be_terminated()
{
	return graphics_should_be_terminated();
}

// TODO: Take the window param into accout
void window_resize_hook(GWindow* window, int width, int height)
{
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

	add_entity(Entity_Triangle, triangle, &tri_pos, &tri_scale, &tri_color);

	return 0;
}

int draw_square_entity(EntityDef** square)
{
	Vec3 sq_pos = { { 900.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 sq_scale = { { 35.f, 35.f, 1.f } };
	Vec4 sq_color = COLOR_VEC_GREEN;

	add_entity(Entity_Square, square, &sq_pos, &sq_scale, &sq_color);

	return 0;
}

int draw_circle_entity(EntityDef** circle)
{
	Vec3 circle_pos = { { 1100.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 circle_scale = { { 35.f, 35.f, 1.f } };
	Vec4 circle_color = COLOR_VEC_GREEN;

	add_entity(Entity_Circle, circle, &circle_pos, &circle_scale, &circle_color);

	return 0;
}

int draw_castle_entity(EntityDef** castle)
{
	Vec3 castle_pos = { { 1600.f, wHeight / 2.f, Z_DEPTH_INITIAL_CASTLE } };
	Vec3 castle_scale = { { 125.f, 125.f, 1.f } };
	Vec4 castle_color = COLOR_VEC_WHITE;

	add_entity(Entity_Castle, castle, &castle_pos, &castle_scale, &castle_color);

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
	add_collision_layer2D(tower_entity->collidable2D->collision_box, CollisionLayer_Tower);
	add_collision_mask2D(tower_entity->collidable2D->collision_box, CollisionLayer_Road);

	draw_circle_entity(&tower_entity);
	towers[1] = tower_entity;

	get_drawable_def(&tower_drawable, tower_entity->drawable_handle);
	tower_drawable->visible = 0;

	// TODO: Add error checks
	add_collidable2D(&tower_entity->collidable2D, &tower_drawable->transform.translation, &tower_drawable->transform.scale);
	add_collision_layer2D(tower_entity->collidable2D->collision_box, CollisionLayer_Tower);
	add_collision_mask2D(tower_entity->collidable2D->collision_box, CollisionLayer_Road);

	draw_triangle_entity(&tower_entity);
	towers[2] = tower_entity;

	get_drawable_def(&tower_drawable, tower_entity->drawable_handle);
	tower_drawable->visible = 0;

	// TODO: Add error checks
	add_collidable2D(&tower_entity->collidable2D, &tower_drawable->transform.translation, &tower_drawable->transform.scale);
	add_collision_layer2D(tower_entity->collidable2D->collision_box, CollisionLayer_Tower);
	add_collision_mask2D(tower_entity->collidable2D->collision_box, CollisionLayer_Road);

	return 0;
}

// TODO: This should be provided by the current map
int create_enemies()
{
	EntityDef* circle = NULL;
	EntityDef* square = NULL;
	EntityDef* triangle = NULL;

	draw_triangle_entity(&triangle);
	draw_square_entity(&square);
	draw_circle_entity(&circle);

	DrawableDef* tri_drawable = NULL;
	get_drawable_def(&tri_drawable, triangle->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(tri_drawable != NULL, "[game]: Failed to fetch triangle drawable.");
	add_collidable2D(&triangle->collidable2D, &tri_drawable->transform.translation, &tri_drawable->transform.scale);
	add_collision_layer2D(triangle->collidable2D->collision_box, CollisionLayer_Enemy);

	const PathDef* path = map_mgr_get_path();
	int path_len = map_mgr_get_path_len();

	add_entity_path(triangle, path, path_len);
	add_entity_path(square, path, path_len);
	add_entity_path(circle, path, path_len);

	enemies[0] = circle;
	enemies[1] = square;
	enemies[2] = triangle;

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
	physics_bind_collision_event_cb(&process_collision_event_hook);

	map_mgr_init();

	int load_map_res = map_mgr_load_map();
	if (TERMINATE_ERR_CODE == load_map_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	create_tower_entities();
	create_enemies();

	draw_castle_entity(&castle);

	DrawableDef* castle_drawable = NULL;
	get_drawable_def(&castle_drawable, castle->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(castle_drawable != NULL, "[game]: Failed to fetch castle drawable.");

	// TODO: Add error checks
	add_collidable2D(&castle->collidable2D, &castle_drawable->transform.translation, &castle_drawable->transform.scale);
	add_collision_layer2D(castle->collidable2D->collision_box, CollisionLayer_Castle);
	add_collision_mask2D(castle->collidable2D->collision_box, CollisionLayer_Enemy);

	const float time_step = 250;
	float path_delay_sq = 250;
	float path_delay_cir = 500;

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

		for (int i = 0; i < ENEMIES_AMOUNT; i++)
		{
			entity_follow_path(enemies[i]);
		}

		//entity_follow_path(triangle);
		//if (path_delay_sq > 0)
		//{
		//	path_delay_sq -= time_step * dt;
		//}
		//else
		//{
		//	entity_follow_path(square);
		//}

		//if (path_delay_cir > 0)
		//{
		//	path_delay_cir -= time_step * dt;
		//}
		//else
		//{
		//	entity_follow_path(circle);
		//}

		if (s_buildingModeEnabled)
		{
			graphics_get_cursor_pos(&cursor_xpos, &cursor_ypos);

			CHECK_EXPR_FAIL_RET_TERMINATE(s_currentTowerIdx < TOWERS_AMOUNT, "[game]: currentTowerIdx is greater than towers amount.");

			EntityDef* tower_entity = towers[s_currentTowerIdx];
			DrawableDef* tower_drawable = NULL;

			get_drawable_def(&tower_drawable, tower_entity->drawable_handle);

			if (NULL != tower_drawable)
			{
				float tower_scale_x = tower_drawable->init_transform.scale.x * wWidth / WINDOW_DEFAULT_RES_W;
				float tower_scale_y = tower_drawable->init_transform.scale.y * wHeight / WINDOW_DEFAULT_RES_H;
				// TODO: Resize all entities
				resize_entity(tower_entity, tower_scale_x, tower_scale_y);
				move_entity(tower_entity, cursor_xpos, wHeight - cursor_ypos);
				// TODO: Add error checking
				move_collision_box2D(tower_entity->collidable2D->collision_box, cursor_xpos, wHeight - cursor_ypos);
			}
		}

		physics_step();
		graphics_draw();
	}

	registry_free();
	map_mgr_free_resources();
	entity_free_resources();
	graphics_free_resources();

	return 0;
}
