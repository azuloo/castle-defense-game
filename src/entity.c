#include "entity.h"
#include "graphics.h"
#include "file_reader.h"
#include "utils.h"
#include "lin_alg.h"

extern float wWidth;
extern float wHeight;

static EntryCnf* add_entity_common(const DrawBufferData* draw_buf_data, const char* texture_path, const Vec3* new_pos, const Vec3* new_scale)
{
	static const char* vertex_shader_path = "/res/static/shaders/entity_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/entity_frag.txt";

	EntryCnf* entry = create_entry();

	char texture_buf[256];
	get_file_path(texture_path, texture_buf, 256);

	int create_texture_2D_res = create_texture_2D(texture_buf, &entry->texture, TexType_RGBA);
	if (TERMINATE_ERR_CODE == create_texture_2D_res)
	{
		PRINT_ERR("[static_env]: Failed to add env texute.");
		return TERMINATE_ERR_CODE;
	}

	int add_res = add_element(entry, draw_buf_data, vertex_shader_path, fragment_shader_path);
	if (TERMINATE_ERR_CODE == add_res)
	{
		PRINT_ERR("[entity]: Failed to add env element.");
		return TERMINATE_ERR_CODE;
	}

	add_entry_attribute(entry, 3);
	add_entry_attribute(entry, 2);

	apply_entry_attributes(entry);

	Mat4 model = IdentityMat;
	scale(&model, new_scale->x, new_scale->y, new_scale->z);
	translate(&model, new_pos->x, new_pos->y, new_pos->z);
	add_uniform_mat4f(entry->shader_prog, "model", &model);

	Mat4 projection;
	projection = ortho(0.f, wWidth, 0.f, wHeight, -1.f, 1.f);
	add_uniform_mat4f(entry->shader_prog, "projection", &projection);
}

static int add_triangle()
{
	static const float vertices[] = {
		// Position           // Texture
		0.f, 1.f, 0.f,       0.5f, 1.f,
		-1.f, -1.f, 0.f,      0.f, 0.f,
		1.f, -1.f, 0.0f,      1.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 2
	};

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices);

	Vec3 tri_pos = { { 600.f, (float)wHeight / 2.f, 0.2f } };
	Vec3 tri_scale = { { 35.f, 35.f, 1.f } };

	const char* texture_path = "/res/static/textures/triangle.png";
	add_entity_common(&draw_buf_data, texture_path, &tri_pos, &tri_scale);

	return 0;
}

static int add_square()
{
	static const float vertices[] = {
		// Position           // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices);

	Vec3 sq_pos = { { 400.f, (float)wHeight / 2.f, 0.2f } };
	Vec3 sq_scale = { { 35.f, 35.f, 1.f } };

	const char* texture_path = "/res/static/textures/square.png";
	add_entity_common(&draw_buf_data, texture_path, &sq_pos, &sq_scale);

	return 0;
}

static int add_circle()
{
	static const float vertices[] = {
		// Position           // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices);

	Vec3 sq_pos = { { 500.f, (float)wHeight / 2.f, 0.2f } };
	Vec3 sq_scale = { { 35.f, 35.f, 1.f } };

	const char* texture_path = "/res/static/textures/circle.png";
	const EntryCnf* entry = add_entity_common(&draw_buf_data, texture_path, &sq_pos, &sq_scale);

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int add_entity(enum EntityType type)
{
	switch (type)
	{
	case Triangle:
		add_triangle();
		break;
	case Square:
		add_square();
		break;
	case Circle:
		add_circle();
		break;
	default:
		PRINT_ERR("[entity]: Unknown entity type.");
		break;
	}

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
