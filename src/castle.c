#include "castle.h"
#include "graphics.h"
#include "utils.h"
#include "global_defs.h"
#include "graphics_defs.h"

// TODO: Move to global_defs
extern float wWidth;
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

	EntryCnf* entry = create_entry();

	char texture_buf[256];
	get_file_path(texture_path, &texture_buf, 256);

	int create_texture_2D_res = create_texture_2D(texture_buf, &entry->texture, TexType_RGB);
	if (TERMINATE_ERR_CODE == create_texture_2D_res)
	{
		PRINT_ERR("[static_env]: Failed to add env texute.");
		return TERMINATE_ERR_CODE;
	}

	int add_res = add_element(entry, &draw_buf_data, vertex_shader_path, fragment_shader_path);
	if (TERMINATE_ERR_CODE == add_res)
	{
		PRINT_ERR("[static_env]: Failed to add env element.");
		return TERMINATE_ERR_CODE;
	}

	add_entry_attribute(entry, 3); // Pos
	add_entry_attribute(entry, 2); // Texture

	apply_entry_attributes(entry);

	entry->matrices->model = IdentityMat;
	scale(&entry->matrices->model, 125.f, 125.f, 1.f);
	translate(&entry->matrices->model, 1500.f, (float)wHeight / 2.f, Z_DEPTH_INITIAL_CASTLE);
	add_uniform_mat4f(entry->shader_prog, "model", &entry->matrices->model);

	entry->matrices->projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(entry->shader_prog, "projection", &entry->matrices->projection);

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
