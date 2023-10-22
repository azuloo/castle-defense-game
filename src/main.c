#include "graphics.h"
#include <stdbool.h>

// TEMP
static float vertices[] = {
	0.5f, -0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
	-0.5f, 0.5f, 0.0f
};

static unsigned int indices[] = {
	1, 3, 2,
	2, 1, 0
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

	draw_triangle(vertices, 12, indices, 6);

	while (!should_be_terminated())
	{
		draw();
	}
	
	graphics_free_resources();

	return 0;
}
