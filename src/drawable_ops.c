#include "drawable_ops.h"
#include "lin_alg.h"

int drawable_transform_ts(DrawableDef* drawable, const char* uniform_name)
{
	drawable->matrices->model = IdentityMat;
	translate(&drawable->matrices->model, drawable->transform->translation.x, drawable->transform->translation.y, drawable->transform->translation.z);
	scale(&drawable->matrices->model, drawable->transform->scale.x, drawable->transform->scale.y, drawable->transform->scale.z);
	add_uniform_mat4f(drawable->shader_prog, uniform_name, &drawable->matrices->model);

	return 0;
}

int get_square_draw_buffer_data(DrawBufferData** dest)
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
