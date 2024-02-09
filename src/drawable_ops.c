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
