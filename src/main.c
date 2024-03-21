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

int wWidth     = WINDOW_DEFAULT_RES_W;
int wHeight    = WINDOW_DEFAULT_RES_H;

float dt       = 0.0f;  // delta time
float lft      = 0.0f;  // last frame time

int s_trackCursorPos = 0;

static void entities_collided_hook(EntityDef* first, EntityDef* second)
{
	DrawableDef* first_drawable = NULL;
	get_drawable_def(&first_drawable, first);

	DrawableDef* second_drawable = NULL;
	get_drawable_def(&second_drawable, second);

	if (NULL == first_drawable || NULL == second_drawable)
	{
		// TODO: Report error
		return;
	}

	// TODO: Move this out of here
	if (NULL != first_drawable && first->type == Entity_Castle)
	{
		Vec4 color_vec = { { 1.f, 0.f, 0.f, 1.f } };
		add_uniform_vec4f(second_drawable->shader_prog, "UColor", &color_vec);
	}
	// TODO: Move this out of here
	if (NULL != second_drawable && second->type == Entity_Castle)
	{
		Vec4 color_vec = { { 1.f, 0.f, 0.f, 1.f } };
		add_uniform_vec4f(first_drawable->shader_prog, "UColor", &color_vec);
	}
}

static void process_key_hook(GWindow* window, int key, int scancode, int action, int mods)
{
	if (key == K_F && action == KEY_PRESS)
	{
		s_trackCursorPos = !s_trackCursorPos;
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
	Vec3 tri_pos = { { 600.f, wHeight / 2.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 tri_scale = { { 35.f, 35.f, 1.f } };
	Vec4 tri_color = { { 0.f, 1.f, 0.f, 1.f } };

	add_entity(Entity_Triangle, triangle, &tri_pos, &tri_scale, &tri_color);
}

int draw_square_entity(EntityDef** square)
{
	Vec3 sq_pos = { { 400.f, wHeight / 2.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 sq_scale = { { 35.f, 35.f, 1.f } };
	Vec4 sq_color = { { 0.f, 1.f, 0.f, 1.f } };

	add_entity(Entity_Square, square, &sq_pos, &sq_scale, &sq_color);
}

int draw_circle_entity(EntityDef** circle)
{
	Vec3 circle_pos = { { 500.f, wHeight / 2.f, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 circle_scale = { { 35.f, 35.f, 1.f } };
	Vec4 circle_color = { { 0.f, 1.f, 0.f, 1.f } };

	add_entity(Entity_Circle, circle, &circle_pos, &circle_scale, &circle_color);
}

int draw_castle_entity(EntityDef** castle)
{
	Vec3 castle_pos = { { 1600.f, wHeight / 2.f, Z_DEPTH_INITIAL_CASTLE } };
	Vec3 castle_scale = { { 125.f, 125.f, 1.f } };
	Vec4 castle_color = { { 1.f, 1.f, 1.f, 1.f } };

	add_entity(Entity_Castle, castle, &castle_pos, &castle_scale, &castle_color);
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
	physics_bind_entities_collided_cb(&entities_collided_hook);

	map_mgr_init();

	int load_map_res = map_mgr_load_map();
	if (TERMINATE_ERR_CODE == load_map_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	EntityDef* castle = NULL;
	draw_castle_entity(&castle);

	EntityDef* circle = NULL;
	EntityDef* square = NULL;
	EntityDef* triangle = NULL;
	EntityDef* building = NULL;

	draw_triangle_entity(&triangle);
	draw_square_entity(&square);
	draw_circle_entity(&circle);
	draw_square_entity(&building);

	const PathSegment** path = map_mgr_get_path();
	int path_len = map_mgr_get_path_len();

	add_entity_path(triangle, path, path_len);
	add_entity_path(square, path, path_len);
	add_entity_path(circle, path, path_len);

	const float time_step = 250;
	float path_delay_sq = 250;
	float path_delay_cir = 500;

	init_ft();
	load_ascii_chars();

	ft_renderer_init();
	Vec3 color = { 1.f, 1.f, 1.f };
	render_text("Sample text", wWidth - 300.f, wHeight - 50.f, color);

	double cursor_xpos = 0;
	double cursor_ypos = 0;

	DrawableDef* drawable = NULL;
	get_drawable_def(&drawable, building);

	// TODO: Handle Windows window drag (other events?)
	while (!should_be_terminated())
	{
		float curr_time = (float)glfwGetTime();
		dt = curr_time - lft;
		lft = curr_time;

		dt = math_clamp(dt, 0.f, MAX_FRAME_TIME);

		entity_follow_path(triangle);
		if (path_delay_sq > 0)
		{
			path_delay_sq -= time_step * dt;
		}
		else
		{
			entity_follow_path(square);
		}

		if (path_delay_cir > 0)
		{
			path_delay_cir -= time_step * dt;
		}
		else
		{
			entity_follow_path(circle);
		}

		if (s_trackCursorPos)
		{
			graphics_get_cursor_pos(&cursor_xpos, &cursor_ypos);

			drawable->transform.scale.x = drawable->init_transform.scale.x * wWidth / WINDOW_DEFAULT_RES_W;
			drawable->transform.scale.y = drawable->init_transform.scale.y * wHeight / WINDOW_DEFAULT_RES_H;

			drawable->transform.translation.x = cursor_xpos;
			drawable->transform.translation.y = wHeight - cursor_ypos;

			drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

			drawable->matrices.projection = COMMON_ORTHO_MAT;
			add_uniform_mat4f(drawable->shader_prog, COMMON_PROJECTION_UNIFORM_NAME, &drawable->matrices.projection);
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
