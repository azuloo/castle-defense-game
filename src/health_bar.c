#include "health_bar.h"
#include "graphics.h"
#include "physics.h"
#include "drawable_ops.h"
#include "global_defs.h"
#include "utils.h"

static const char* s_HealthBarOuterTexture = "/res/static/textures/health_bar.png";
static const char* s_quad_vertex_shader_path = "/res/static/shaders/basic_vert.txt";
static const char* s_quad_frag_shader_path = "/res/static/shaders/health_bar_frag.txt";

static HealthBarDef* s_HealthBar = NULL;
static int s_HealthBarCapacity = 1;
static int s_HealthBarCount = 0;

static int alloc_heath_bar_arr()
{
	s_HealthBarCapacity *= 2;
	HealthBarDef* health_bar = realloc(s_HealthBar, s_HealthBarCapacity * sizeof *health_bar);
	CHECK_EXPR_FAIL_RET_TERMINATE(health_bar != NULL, "[health_bar]: Failed to allocate enough memory for health bar arr.");

	s_HealthBar = health_bar;

	return 0;
}

static int create_health_bar(HealthBarDef** dest)
{
	if (NULL == s_HealthBar || s_HealthBarCount >= s_HealthBarCapacity)
	{
		int alloc_health_bar_arr_res = alloc_heath_bar_arr();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != alloc_health_bar_arr_res, "[health_bar]: Failed to create health bar arr.");
	}

	HealthBarDef* health_bar = s_HealthBar + s_HealthBarCount;
	health_bar->collision_box = NULL;
	health_bar->value = HEALTH_BAR_DEFAULT_VALUE;
	health_bar->drawable_handle = -1;

	s_HealthBarCount++;
	*dest = health_bar;

	return 0;
}

int add_health_bar(const Vec3* pos, const Vec3* scale)
{
	const Vec4 color = { { 1.f, 1.f, 1.f, 1.f } };
	DrawableDef* drawable = NULL;

	create_drawable(&drawable);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[health_bar]: Drawable is not defined.");

	static const int quad_texture_params[DEFAULT_TEXTURE_PARAMS_COUNT] = {
		GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER,
		GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER,
		GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR,
		GL_TEXTURE_MAG_FILTER, GL_LINEAR
	};
	add_texture_2D(drawable, s_HealthBarOuterTexture, TexType_RGBA, quad_texture_params, sizeof(quad_texture_params) / sizeof(quad_texture_params[0]));

	DrawBufferData* draw_buf_data = NULL;
	get_quad_draw_buffer_data(&draw_buf_data);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != draw_buf_data, "[health_bar]: Failed to retrieve square DrawBufferData.");

	int add_res = setup_drawable(drawable, draw_buf_data, s_quad_vertex_shader_path, s_quad_frag_shader_path);
	CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != add_res, "[health_bar]: Failed to add env element.");

	// TODO: Replace with common method (e.g. draw_quad)
	register_drawable_attribute(drawable, POS_TEXTURE_ATTRIBUTE_SIZE); // Position + texture.
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

	HealthBarDef* health_bar = NULL;
	create_health_bar(&health_bar);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != health_bar, "[health_bar]: Failed to create a health bar.");

	health_bar->drawable_handle = drawable->handle;

	return 0;
}

void health_bar_free_resources()
{
	if (NULL != s_HealthBar)
	{
		free(s_HealthBar);
	}
}
