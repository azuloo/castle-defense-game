#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "key_bindings.h"

#include "static_env.h"
#include "entity.h"

#include <string.h>

float wWidth = WINDOW_DEFAULT_RES_W;
float wHeight = WINDOW_DEFAULT_RES_H;

float dt = 0.0f;     // delta time
float lft = 0.0f;    // last frame time

static int should_be_terminated()
{
	return graphics_should_be_terminated();
}

// TODO: Take the window param into accout
void window_resize_hook(GWindow* window, float width, float height)
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

	int add_env_res = add_environments();
	if (TERMINATE_ERR_CODE == add_env_res)
	{
		APP_EXIT(TERMINATE_ERR_CODE);
	}

	EntityDef* triangle = NULL;
	EntityDef* square = NULL;
	EntityDef* circle = NULL;

	add_entity(Triangle, &triangle);
	add_entity(Square, &square);
	add_entity(Circle, &circle);

	while (!should_be_terminated())
	{
		float curr_time = (float)glfwGetTime();
		dt = curr_time - lft;
		lft = curr_time;

		draw();
	}
	
	graphics_free_resources();

	return 0;
}
