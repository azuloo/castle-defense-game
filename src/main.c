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

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = 32;
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = 6;

	EntryCnf* entry = create_entry();
	draw_triangle(entry, &draw_buf_data);

	while (!should_be_terminated())
	{
		draw();
	}
	
	graphics_free_resources();

	return 0;
}
