#include "graphics.h"
#include <stdbool.h>

// TEMP
static float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.0f, 0.5f, 0.0f
};

static float vertices2[] = {
	-0.5f, -0.25f, 0.0f,
	0.5f, -0.25f, 0.0f,
	0.0f, 0.75f, 0.0f
};

static float vertices3[] = {
	-0.5f, -0.75f, 0.0f,
	0.5f, -0.75f, 0.0f,
	0.0f, 0.25f, 0.0f
};

static bool should_be_terminated()
{
	return graphics_should_be_terminated();
}

int main(int argc, int* argv[])
{
	init_graphics();

	BackgroundColor b_Color;
	b_Color.R = 0.3f;
	b_Color.G = 0.4f;
	b_Color.B = 0.5f;
	b_Color.A = 1.0f;

	set_background_color(b_Color);

	draw_triangle(vertices, 9);
	draw_triangle(vertices2, 9);
	draw_triangle(vertices3, 9);

	while (!should_be_terminated())
	{
		draw();
	}
	
	graphics_free_resources();

	return 0;
}
