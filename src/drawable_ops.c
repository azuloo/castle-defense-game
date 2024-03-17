#include "drawable_ops.h"
#include "global_defs.h"
#include "graphics_defs.h"
#include "file_reader.h"
#include "lin_alg.h"

#define POS_ATTRIBUTE_SIZE 3
#define TEXTURE_ATTRIBUTE_SIZE 2
#define SHADER_COLOR_UNIFORM_NAME "UColor"

static const char* s_quad_vertex_shader_path   = "/res/static/shaders/basic_vert.txt";
static const char* s_quad_frag_shader_path     = "/res/static/shaders/quad_frag.txt";

int drawable_transform_ts(DrawableDef* drawable, const char* uniform_name)
{
	drawable->matrices.model = IdentityMat;
	translate(&drawable->matrices.model, drawable->transform.translation.x, drawable->transform.translation.y, drawable->transform.translation.z);
	scale(&drawable->matrices.model, drawable->transform.scale.x, drawable->transform.scale.y, drawable->transform.scale.z);
	add_uniform_mat4f(drawable->shader_prog, uniform_name, &drawable->matrices.model);

	return 0;
}

int get_quad_draw_buffer_data(DrawBufferData** dest)
{
	static float vertices[] = {
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

	// TODO: Is it OK to have entities share the same static draw buf data? (causes no prolems for now)
	static DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = SQUARE_VERTICES_LEN;
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = SQUARE_INDICES_LEN;

	*dest = &draw_buf_data;

	return 0;
}

int draw_quad(DrawableDef** dest, const char* texture_path, int texture_type, const Vec3* new_pos, const Vec3* new_scale, const Vec4* new_color)
{
	DrawableDef* drawable = create_drawable();
	if (NULL == drawable)
	{
		PRINT_ERR("[drawable_ops]: Failed to create drawable.");
		return TERMINATE_ERR_CODE;
	}

	*dest = drawable;

	// TODO: Common code - move to separate function
	char path_buf[256];
	get_file_path(texture_path, &path_buf, 256);

	unsigned char* img_data;
	int width, height;
	fr_read_image_data(path_buf, &img_data, &width, &height);

	int create_texture_2D_res = create_texture_2D(img_data, width, height, &drawable->texture, texture_type);
	fr_free_image_resources(img_data);

	if (TERMINATE_ERR_CODE == create_texture_2D_res)
	{
		PRINT_ERR("[drawable_ops]: Failed to add env texute.");
		return TERMINATE_ERR_CODE;
	}

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

	register_drawable_attribute(drawable, POS_ATTRIBUTE_SIZE);       // Pos
	register_drawable_attribute(drawable, TEXTURE_ATTRIBUTE_SIZE);   // Texture

	process_drawable_attributes(drawable);

	drawable->transform.translation   = *new_pos;
	drawable->transform.scale         = *new_scale;

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

	drawable->matrices.projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(drawable->shader_prog, COMMON_PROJECTION_UNIFORM_NAME, &drawable->matrices.projection);

	add_uniform_vec4f(drawable->shader_prog, SHADER_COLOR_UNIFORM_NAME, new_color);

	return 0;
}
