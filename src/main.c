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

#include "map/map_mgr.h"
#include "entity.h"

#include <string.h>

int wWidth     = WINDOW_DEFAULT_RES_W;
int wHeight    = WINDOW_DEFAULT_RES_H;

float dt       = 0.0f;  // delta time
float lft      = 0.0f;  // last frame time

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

int main(int argc, int* argv[])
{
	int init_graphics_res = init_graphics();
	if (TERMINATE_ERR_CODE == init_graphics_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	bind_input_fn(&process_input);
	bind_window_resize_fn(&window_resize_hook);

	map_mgr_init();

	int load_map_res = map_mgr_load_map();
	if (TERMINATE_ERR_CODE == load_map_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	EntityDef* triangle = NULL;
	EntityDef* square = NULL;
	EntityDef* circle = NULL;

	add_entity(Entity_Triangle, &triangle);
	add_entity(Entity_Square, &square);
	add_entity(Entity_Circle, &circle);

	const PathSegment** path = map_mgr_get_path();
	int path_len = map_mgr_get_path_len();

	add_entity_path(triangle, path, path_len);
	add_entity_path(square, path, path_len);
	add_entity_path(circle, path, path_len);

	const float time_step = 250;
	float path_delay_sq = 250;
	float path_delay_cir = 500;

	EntityDef* castle = NULL;
	add_entity(Entity_Castle, &castle);

	init_ft();
	load_ascii_chars();

	ft_renderer_init();
	Vec3 color = { 1.f, 1.f, 1.f };
	render_text("Sample text", wWidth - 300.f, wHeight - 50.f, color);

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

		physics_step();
		graphics_draw();
	}

	registry_free();
	map_mgr_free_resources();
	entity_free_resources();
	graphics_free_resources();

	return 0;
}
