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
	load_ascii_chars();

	Vec3 color = { 1.f, 1.f, 1.f };
	render_text("Press 1, 2 or 3 to select Towers", wWidth - 400.f, wHeight - 50.f, color);

	float tower_x_pos = 0.f;
	float tower_y_pos = 0.f;

	sort_drawables();
	// TODO: Handle Windows window drag (other events?)
	while (!should_be_terminated())
	{
		float curr_time = (float)glfwGetTime();
		dt = curr_time - lft;
		lft = curr_time;

		dt = math_clamp(dt, 0.f, MAX_FRAME_TIME);

		enemy_waves_spawn(dt);
		update_towers(dt);

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
