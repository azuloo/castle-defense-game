#include "health_bar.h"
#include "graphics.h"
#include "drawable_ops.h"
#include "global_defs.h"
#include "graphics_defs.h"
#include "utils.h"

static const char* s_HealthBarOuterTexture = "/res/static/textures/health_bar.png";
static const char* s_quad_vertex_shader_path = "/res/static/shaders/basic_vert.txt";
static const char* s_quad_frag_shader_path = "/res/static/shaders/health_bar_frag.txt";

int add_health_bar(const Vec3* pos, const Vec3* scale)
{
	const Vec4 color = { { 1.f, 1.f, 1.f, 1.f } };
	DrawableDef* drawable = NULL;

	create_drawable(&drawable);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[health_bar]: Drawable is not defined.");

	add_texture_2D(drawable, s_HealthBarOuterTexture, TexType_RGBA);

	DrawBufferData* draw_buf_data = NULL;
	get_quad_draw_buffer_data(&draw_buf_data);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != draw_buf_data, "[health_bar]: Failed to retrieve square DrawBufferData.");

	int add_res = setup_drawable(drawable, draw_buf_data, s_quad_vertex_shader_path, s_quad_frag_shader_path);
	CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != add_res, "[health_bar]: Failed to add env element.");

	register_drawable_attribute(drawable, POS_TEXTURE_ATTRIBUTE_SIZE);       // Position + texture.
	process_drawable_attributes(drawable);

	drawable->transform.translation = *pos;
	drawable->transform.scale = *scale;

	drawable->init_transform.translation = *pos;
	drawable->init_transform.scale = *scale;

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

	drawable->matrices.projection = COMMON_ORTHO_MAT;
	add_uniform_mat4f(drawable->shader_prog, COMMON_PROJECTION_UNIFORM_NAME, &drawable->matrices.projection);

	add_uniform_vec4f(drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color);
	add_uniform_1f(drawable->shader_prog, "uFilled", 1.f);

	return 0;
}
