#include "static_env.h"

extern float wWidth;
extern float wHeight;

int add_environments()
{
	static const float B_vertices[] = {
		// Position            // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	static unsigned int B_indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	static const char* vertex_shader_path = "/res/static/shaders/field_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/field_frag.txt";
	static const char* texure_name = "/res/static/textures/field.jpg";

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = B_vertices;
	draw_buf_data.vertices_len = 20;
	draw_buf_data.indices = B_indices;
	draw_buf_data.indices_len = 6;

	EntryCnf* entry = create_entry();

	char texture_path[256];
	get_file_path(texure_name, texture_path, 256);

	int create_texture_2D_res = create_texture_2D(texture_path, &entry->texture);
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

	add_entry_attribute(entry, 3);
	add_entry_attribute(entry, 2);

	apply_entry_attributes(entry);

	Mat4 model = IdentityMat;
	scale(&model, (float)wWidth / 2, (float)wHeight / 2, 1.f);
	translate(&model, (float)wWidth / 2, (float)wHeight / 2, 1.f);
	add_uniform_mat4f(entry->shader_prog, "model", &model);

	Mat4 projection;
	projection = ortho(0.f, wWidth, wHeight, 0.f, -1.f, 1.f);
	add_uniform_mat4f(entry->shader_prog, "projection", &projection);
}
