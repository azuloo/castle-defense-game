#include "graphics.h"
#include "file_reader.h"
#include "utils.h"

#include <string.h>

// TEMP
static float vertices[] = {
	// Position            // Color             // Texture
	0.5f, -0.5f, 0.0f,     0.5f, 0.5f, 0.0f,    1.0f, 0.0f,
	-0.5f, -0.5f, 0.0f,    0.0f, 0.5f, 0.5f,    0.0f, 0.0f,
	0.5f, 0.5f, 0.0f,      0.5f, 0.0f, 0.5f,    1.0f, 1.0f,
	-0.5f, 0.5f, 0.0f,     0.5f, 0.5f, 0.5f,    0.0f, 1.0f
};

static unsigned int indices[] = {
	1, 3, 2,
	2, 1, 0
};

static int should_be_terminated()
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

	ArrayUnit castle_vertices;
	castle_vertices.data = vertices;
	castle_vertices.len = 32;

	ArrayUnit castle_indices;
	castle_indices.data = indices;
	castle_indices.len = 6;

	EntryCnf* entry = create_entry(vertices, 32);
	draw_triangle(entry, &castle_vertices, &castle_indices);

	while (!should_be_terminated())
	{
		draw();
	}
	
	graphics_free_resources();

	return 0;
}
