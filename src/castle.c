#include "castle.h"
#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "global_defs.h"
#include "graphics_defs.h"

extern float wHeight;

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int add_castle()
{
	// TODO: Rect verices are identical; move somewhere
	static float vertices[] = {
		// Position            // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	static const char* vertex_shader_path = "/res/static/shaders/basic_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/basic_frag.txt";
	static const char* texture_path = "/res/static/textures/base.jpg";

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices) / sizeof(vertices[0]);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices) / sizeof(indices[0]);

	DrawableDef* drawable = create_drawable();

	char texture_buf[256];
	get_file_path(texture_path, &texture_buf, 256);

	int create_texture_2D_res = create_texture_2D(texture_buf, &drawable->texture, TexType_RGB);
	if (TERMINATE_ERR_CODE == create_texture_2D_res)
	{
		PRINT_ERR("[static_env]: Failed to add env texute.");
		return TERMINATE_ERR_CODE;
	}

	int add_res = config_drawable(drawable, &draw_buf_data, vertex_shader_path, fragment_shader_path);
	if (TERMINATE_ERR_CODE == add_res)
	{
		PRINT_ERR("[static_env]: Failed to add env element.");
		return TERMINATE_ERR_CODE;
	}

	register_drawable_attribute(drawable, 3); // Pos
	register_drawable_attribute(drawable, 2); // Texture

	process_drawable_attributes(drawable);

	drawable->matrices->model = IdentityMat;
	scale(&drawable->matrices->model, 125.f, 125.f, 1.f);
	translate(&drawable->matrices->model, 1500.f, wHeight / 2.f, Z_DEPTH_INITIAL_CASTLE);
	add_uniform_mat4f(drawable->shader_prog, "model", &drawable->matrices->model);

	drawable->matrices->projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(drawable->shader_prog, "projection", &drawable->matrices->projection);

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
