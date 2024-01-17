#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "key_bindings.h"
#include "obj_registry.h"

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

	add_entity(Entity_Triangle, &triangle);
	add_entity(Entity_Square, &square);
	add_entity(Entity_Circle, &circle);

	PathSegment path[4] = {
		[0] = { .start = { 200.f, 200.f }, .end = { 200.f, 800.f } },
		[1] = { .start = { 200.f, 800.f }, .end = { 800.f, 800.f } },
		[2] = { .start = { 800.f, 800.f }, .end = { 800.f, 200.f } },
		[3] = { .start = { 800.f, 200.f }, .end = { 200.f, 200.f } },
	};

	add_entity_path(triangle, path, 4);

	while (!should_be_terminated())
	{
		float curr_time = (float)glfwGetTime();
		dt = curr_time - lft;
		lft = curr_time;

		entity_follow_path(triangle);
		draw();
	}

	for (int i = 0; i < triangle->path_len; i++)
	{
		free(triangle->path[i]);
	}
	free(triangle->path);
	free(triangle->transform);
	free(triangle);

	for (int i = 0; i < square->path_len; i++)
	{
		free(square->path[i]);
	}
	free(square->path);
	free(square->transform);
	free(square);

	for (int i = 0; i < circle->path_len; i++)
	{
		free(circle->path[i]);
	}
	free(circle->path);
	free(circle->transform);
	free(circle);

	registry_free();
	graphics_free_resources();

	return 0;
}
