#include "drawable_ops.h"
#include "global_defs.h"
#include "graphics_defs.h"
#include "file_reader.h"
#include "lin_alg.h"

#define SHADER_COLOR_UNIFORM_NAME "UColor"

static const char* s_quad_vertex_shader_path   = "/res/static/shaders/basic_vert.txt";
static const char* s_quad_frag_shader_path     = "/res/static/shaders/quad_frag.txt";

int drawable_transform_ts(DrawableDef* drawable, const char* uniform_name)
{
	drawable->matrices.model = IdentityMat;
	translate(&drawable->matrices.model, drawable->transform.translation.x, drawable->transform.translation.y, drawable->transform.translation.z);
	scale(&drawable->matrices.model, drawable->transform.scale.x, drawable->transform.scale.y, drawable->transform.scale.z);
	add_uniform_mat4f(drawable->shader_prog, uniform_name, &drawable->matrices.model);

	drawable->matrices.projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(drawable->shader_prog, COMMON_PROJECTION_UNIFORM_NAME, &drawable->matrices.projection);

	return 0;
}

int get_quad_draw_buffer_data(DrawBufferData** dest)
{
	static float vertices[] = {
		// Position    // Texture
		-1.f, 1.f,     0.f, 1.f,
		1.f, 1.f,      1.f, 1.f,
		1.f, -1.f,     1.f, 0.f,
		-1.f, -1.f,    0.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	static DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices);

	*dest = &draw_buf_data;

	return 0;
}

// TODO: Remove required texture_path and texture_type and move it into separate func?
int draw_quad(DrawableDef** dest, const char* texture_path, int texture_type, const Vec3* new_pos, const Vec3* new_scale, const Vec4* new_color)
{
	DrawableDef* drawable = create_drawable();
	if (NULL == drawable)
	{
		PRINT_ERR("[drawable_ops]: Failed to create drawable.");
		return TERMINATE_ERR_CODE;
	}

	*dest = drawable;

	add_texture_2D(drawable, texture_path, texture_type);

	DrawBufferData* draw_buf_data = NULL;
	get_quad_draw_buffer_data(&draw_buf_data);
	if (NULL == draw_buf_data)
	{
		PRINT_ERR("[drawable_ops]: Failed to retrieve square DrawBufferData.");
		return TERMINATE_ERR_CODE;
	}

	int add_res = setup_drawable(drawable, draw_buf_data, s_quad_vertex_shader_path, s_quad_frag_shader_path);
	if (TERMINATE_ERR_CODE == add_res)
	{
		PRINT_ERR("[drawable_ops]: Failed to add env element.");
		return TERMINATE_ERR_CODE;
	}

	register_drawable_attribute(drawable, POS_TEXTURE_ATTRIBUTE_SIZE);       // Position + texture.
	process_drawable_attributes(drawable);

	drawable->transform.translation   = *new_pos;
	drawable->transform.scale         = *new_scale;

	drawable->init_transform.translation   = *new_pos;
	drawable->init_transform.scale         = *new_scale;

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

	drawable->matrices.projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(drawable->shader_prog, COMMON_PROJECTION_UNIFORM_NAME, &drawable->matrices.projection);

	add_uniform_vec4f(drawable->shader_prog, SHADER_COLOR_UNIFORM_NAME, new_color);

	return 0;
}
