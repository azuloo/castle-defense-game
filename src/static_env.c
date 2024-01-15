#include "static_env.h"
#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"

extern float wWidth;
extern float wHeight;

// ----------------------- PUBLIC FUNCTIONS ----------------------- //
// TODO: Add EntityDef** dest arg
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
	static const char* texture_path = "/res/static/textures/field.jpg";

	// TODO: Pass by ref below; should allocate on heap?
	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = B_vertices;
	draw_buf_data.vertices_len = sizeof(B_vertices);
	draw_buf_data.indices = B_indices;
	draw_buf_data.indices_len = sizeof(B_indices);

	EntryCnf* entry = create_entry();

	char texture_buf[256];
	get_file_path(texture_path, texture_buf, 256);

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

	add_entry_attribute(entry, 3);
	add_entry_attribute(entry, 2);

	apply_entry_attributes(entry);

	GMatrices* matrices = malloc(sizeof *matrices);
	if (NULL == matrices)
	{
		PRINT_ERR("[static_env]: Failed to allocate sufficient memory chunk for GMatrices.");
		return TERMINATE_ERR_CODE;
	}

	matrices->model = IdentityMat;
	scale(&matrices->model, (float)wWidth / 2, (float)wHeight / 2, 1.f);
	translate(&matrices->model, (float)wWidth / 2, (float)wHeight / 2, 0.1f);
	add_uniform_mat4f(entry->shader_prog, "model", &matrices->model);

	matrices->projection = ortho(0.f, wWidth, 0.f, wHeight, -1.f, 1.f);
	add_uniform_mat4f(entry->shader_prog, "projection", &matrices->projection);
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
