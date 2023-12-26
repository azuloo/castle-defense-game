#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"
#include "key_bindings.h"

#include <string.h>

// TEMP
static float vertices[] = {
	// Position            // Texture
	0.5f, -0.5f, 0.0f,     1.0f, 0.0f,
	-0.5f, -0.5f, 0.0f,    0.0f, 0.0f,
	0.5f, 0.5f, 0.0f,      1.0f, 1.0f,
	-0.5f, 0.5f, 0.0f,     0.0f, 1.0f
};

float wWidth = WINDOW_DEFAULT_RES_W;
float wHeight = WINDOW_DEFAULT_RES_H;

Vec4 camPos = { { 0.f, 0.f, 3.f } };
Vec4 camDir = { { 0.f, 0.f, -1.f } };
Vec4 camUp = { { 0.f, 1.f, 0.f, 0.f } };

float dt = 0.0f;     // delta time
float lft = 0.0f;    // last frame time

static unsigned int indices[] = {
	1, 3, 2,
	2, 1, 0
};

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

	const float camSpeed = dt * 2.5f;
	if (KEY_PRESSED(window, K_SPACE))
	{
		camPos = add(camPos, multipty_by_scalar(camUp, camSpeed));
	}
	if (KEY_PRESSED(window, K_BACKSPACE))
	{
		camPos = sub(camPos, multipty_by_scalar(camUp, camSpeed));
	}
	if (KEY_PRESSED(window, K_W))
	{
		camPos = add(camPos, multipty_by_scalar(camDir, camSpeed));
	}
	if (KEY_PRESSED(window, K_S))
	{
		camPos = sub(camPos, multipty_by_scalar(camDir, camSpeed));
	}
	if (KEY_PRESSED(window, K_A))
	{
		Vec4 camSide = cross(camDir, camUp);
		normaliz_vec4(&camSide);
		camPos = sub(camPos, multipty_by_scalar(camSide, camSpeed));
	}
	if (KEY_PRESSED(window, K_D))
	{
		Vec4 camSide = cross(camDir, camUp);
		normaliz_vec4(&camSide);
		camPos = add(camPos, multipty_by_scalar(camSide, camSpeed));
	}
}

int main(int argc, int* argv[])
{
	init_graphics();
	bind_input_fn(&process_input);
	bind_window_resize_fn(&window_resize_hook);

	BackgroundColor b_Color;
	b_Color.R = 0.3f;
	b_Color.G = 0.4f;
	b_Color.B = 0.5f;
	b_Color.A = 1.0f;

	set_background_color(b_Color);

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = 20;
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = 6;

	EntryCnf* entry = create_entry();

	const char* texure_name = "/res/brick.jpg";
	char texture_path[256];
	get_file_path(texure_name, texture_path, 256);
	create_texture_2D(texture_path, &entry->texture);

	add_element(entry, &draw_buf_data);
	add_entry_attribute(entry, 3);
	add_entry_attribute(entry, 2);

	apply_entry_attributes(entry);

	Mat4 model = IdentityMat;
	add_uniform_mat4f(entry->shader_prog, "model", &model);

	while (!should_be_terminated())
	{
		float curr_time = (float)glfwGetTime();
		dt = curr_time - lft;
		lft = curr_time;

		Mat4 projection;
		float ar = wWidth / wHeight;
		projection = ortho(-ar, ar, -1.f, 1.f, 0.1f, 100.f);
		add_uniform_mat4f(entry->shader_prog, "projection", &projection);

		Mat4 view;
		view = look_at(camPos, add(camPos, camDir), camUp);
		add_uniform_mat4f(entry->shader_prog, "view", &view);

		draw();
	}
	
	graphics_free_resources();

	return 0;
}
