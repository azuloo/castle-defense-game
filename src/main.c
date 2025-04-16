#include "graphics.h"
#include "physics.h"
#include "utils.h"
#include "lin_alg.h"
#include "key_bindings.h"
#include "freetype_text.h"
#include "freetype_renderer.h"
#include "global_defs.h"
#include "health_bar.h"
#include "tower.h"
#include "hud.h"
#include "player_controls.h"

#include "map/map_mgr.h"
#include "entity.h"
#include "enemy_wave.h"

#include <string.h>
#include <stdbool.h>

int xWOffset           = 0;
int yWOffset           = 0;
int wWidth             = WINDOW_DEFAULT_RES_W;
int wHeight            = WINDOW_DEFAULT_RES_H;

float dt   = 0.0f;  // Delta time.
float lft  = 0.0f;  // Last frame time.

extern float get_window_scale_x();
extern float get_window_scale_y();

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

void free_sub_systems_resources()
{
	enemy_waves_free_resources();
	map_mgr_free_resources();
	entity_free_resources();
	graphics_free_resources();
	physics_free_resources();
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
	bind_player_controls();

	map_mgr_init();
	map_mgr_add_castle();

	int load_map_res = map_mgr_load_map();
	if (TERMINATE_ERR_CODE == load_map_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	init_entity();
	init_towers();

	init_ft();
	add_font(FT_DEFAULT_FONT_SIZE);
	add_font(48);
	load_ascii_chars();

	render_player_controls();

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
		update_towers(dt);

		process_player_controls();

		sort_drawables();

		physics_step();
		graphics_draw();
	}

	free_sub_systems_resources();

	return 0;
}
