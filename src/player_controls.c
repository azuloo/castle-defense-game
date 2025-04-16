#include "player_controls.h"
#include "key_bindings.h"
#include "graphics.h"
#include "tower.h"

static int s_BuildingModeEnabled = 0;

double s_CursorXPos = 0.f;
double s_CursorYPos = 0.f;

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

void bind_player_controls()
{
	bind_key_pressed_cb(&process_key_hook);
	bind_mouse_button_cb(&process_mouse_button_hook);
}

void process_player_controls()
{
	if (s_BuildingModeEnabled)
	{
		graphics_get_cursor_pos(&s_CursorXPos, &s_CursorYPos);
		on_tower_building_mode_enabled();
	}
}
