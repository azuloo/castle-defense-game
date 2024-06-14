#include "lin_alg.h"
#include <string.h>

static const double PI = 3.14159265358979323846;

float math_min(float a, float b)
{
	return a > b ? b : a;
}

float math_max(float a, float b)
{
	return a > b ? a : b;
}

float math_clamp(float a, float min_val, float max_val)
{
	return math_min(math_max(a, min_val), max_val);
}

float math_lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

Vec2 vec2_multiply_by_scalar(float scalar, Vec2 v)
{
	return (Vec2) { scalar * v.x, scalar * v.y };
}

bool vec2_equals(Vec2 v1, Vec2 v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

Mat4 multiply_mat4(const Mat4* m1, const Mat4* m2)
{
	Mat4 out = IdentityMat;
	unsigned int row, column, row_offset;

	for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
	{
		for (column = 0; column < 4; ++column)
		{
			out.m[row_offset + column] =
				(m1->m[row_offset + 0] * m2->m[column + 0]) +
				(m1->m[row_offset + 1] * m2->m[column + 4]) +
				(m1->m[row_offset + 2] * m2->m[column + 8]) +
				(m1->m[row_offset + 3] * m2->m[column + 12]);
		}
	}


	return out;
}

Vec4 mulmat_vec4(const Mat4* m, const Vec4* v)
{
	Vec4 out;
	for (int i = 0; i < 4; ++i)
	{
		out.m[i] =
			(v->m[0] * m->m[i + 0]) +
			(v->m[1] * m->m[i + 4]) +
			(v->m[2] * m->m[i + 8]) +
			(v->m[3] * m->m[i + 12]);
	}

	return out;
}

void normaliz_vec4(Vec4* v)
{
	float sqr = v->m[0] * v->m[0] + v->m[1] * v->m[1] + v->m[2] * v->m[2];
	if (sqr == 1 || sqr == 0)
	{
		return;
	}
	float invrt = 1.f / (float)sqrt(sqr);
	v->m[0] *= invrt;
	v->m[1] *= invrt;
	v->m[2] *= invrt;
}

void normaliz_vec3(Vec3* v)
{
	float sqr = v->m[0] * v->m[0] + v->m[1] * v->m[1];
	if (sqr == 1 || sqr == 0)
	{
		return;
	}
	float invrt = 1.f / (float)sqrt(sqr);
	v->m[0] *= invrt;
	v->m[1] *= invrt;
}

Vec4 sub_vec4(Vec4 v1, Vec4 v2)
{
	Vec4 out = { {0} };
	out.m[0] = v1.m[0] - v2.m[0];
	out.m[1] = v1.m[1] - v2.m[1];
	out.m[2] = v1.m[2] - v2.m[2];
	out.m[3] = v1.m[3] - v2.m[3];
	return out;
}

Vec4 add_vec4(Vec4 v1, Vec4 v2)
{
	Vec4 out = { {0} };
	out.m[0] = v1.m[0] + v2.m[0];
	out.m[1] = v1.m[1] + v2.m[1];
	out.m[2] = v1.m[2] + v2.m[2];
	out.m[3] = v1.m[3] + v2.m[3];
	return out;
}

Vec3 sub_vec3(Vec3 v1, Vec3 v2)
{
	Vec3 out = { {0} };
	out.m[0] = v1.m[0] - v2.m[0];
	out.m[1] = v1.m[1] - v2.m[1];
	out.m[2] = v1.m[2] - v2.m[2];
	return out;
}

Vec3 add_vec3(Vec3 v1, Vec3 v2)
{
	Vec3 out = { {0} };
	out.m[0] = v1.m[0] + v2.m[0];
	out.m[1] = v1.m[1] + v2.m[1];
	out.m[2] = v1.m[2] + v2.m[2];
	return out;
}

Vec4 multipty_by_scalar(Vec4 v, float s)
{
	Vec4 out = { {0} };
	out.m[0] = s * v.m[0];
	out.m[1] = s * v.m[1];
	out.m[2] = s * v.m[2];
	out.m[3] = s * v.m[3];
	return out;
}

float dot_vec4(Vec4 v1, Vec4 v2)
{
	return v1.m[0] * v2.m[0] + v1.m[1] * v2.m[1] + v1.m[2] * v2.m[2] + v1.m[3] * v2.m[3];
}

Vec4 cross_vec4(Vec4 v1, Vec4 v2)
{
	Vec4 out = { {0} };
	out.m[0] = v1.m[1] * v2.m[2] - v1.m[2] * v2.m[1];
	out.m[1] = v1.m[2] * v2.m[0] - v1.m[0] * v2.m[2];
	out.m[2] = v1.m[0] * v2.m[1] - v1.m[1] * v2.m[0];
	return out;
}

void rotate(const Mat4* m, const Vec3* axis, float angle)
{
	Mat4 rotation = IdentityMat;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);
	const double del_cos = 1.0 - cosine;
	const double xTs = (double)axis->x * sine;
	const double yTs = (double)axis->y * sine;
	const double zTs = (double)axis->z * sine;

	rotation.m[0] = cosine + (pow(axis->x, 2) * del_cos);
	rotation.m[1] = (axis->x * axis->y * del_cos) - zTs;
	rotation.m[2] = (axis->x * axis->z * del_cos) + yTs;
	rotation.m[4] = (axis->y * axis->x * del_cos) + zTs;
	rotation.m[5] = cosine + (pow(axis->y, 2) * del_cos);
	rotation.m[6] = (axis->y * axis->z * del_cos) - xTs;
	rotation.m[8] = (axis->z * axis->x * del_cos) - yTs;
	rotation.m[9] = (axis->z * axis->y * del_cos) + xTs;
	rotation.m[10] = cosine + (pow(axis->z, 2) * del_cos);

	memcpy(m->m, multiply_mat4(&rotation, m).m, sizeof(m->m));
}

void rotate_x(const Mat4* m, float angle)
{
	Mat4 rotation = IdentityMat;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);

	rotation.m[5] = cosine;
	rotation.m[6] = -sine;
	rotation.m[9] = sine;
	rotation.m[10] = cosine;

	memcpy(m->m, multiply_mat4(&rotation, m).m, sizeof(m->m));
}

void rotate_y(const Mat4* m, float angle)
{
	Mat4 rotation = IdentityMat;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);

	rotation.m[0] = cosine;
	rotation.m[8] = sine;
	rotation.m[2] = -sine;
	rotation.m[10] = cosine;

	memcpy(m->m, multiply_mat4(&rotation, m).m, sizeof(m->m));
}

void rotate_z(const Mat4* m, float angle)
{
	Mat4 rotation = IdentityMat;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);

	rotation.m[0] = cosine;
	rotation.m[1] = -sine;
	rotation.m[4] = sine;
	rotation.m[5] = cosine;

	memcpy(m->m, multiply_mat4(&rotation, m).m, sizeof(m->m));
}

void scale(const Mat4* m, float x, float y, float z)
{
	Mat4 scale = IdentityMat;

	scale.m[0] = x;
	scale.m[5] = y;
	scale.m[10] = z;

	memcpy(m->m, multiply_mat4(&scale, m).m, sizeof(m->m));
}

void translate(const Mat4* m, float x, float y, float z)
{
	Mat4 translation = IdentityMat;

	translation.m[12] = x;
	translation.m[13] = y;
	translation.m[14] = z;

	memcpy(m->m, multiply_mat4(&translation, m).m, sizeof(m->m));
}

float degrees(float radians)
{
	return radians * (180.f / (float)PI);
}

float radians(float degrees)
{
	return degrees * ((float)PI / 180.f);
}

Mat4 perspective(float fovy, float aspect_ratio, float near_plane, float far_plane)
{
	Mat4 out = { { 0 } };

	const float rad = fovy;
	const float tan_half_fovy = (float)tan(rad / 2.f);

	out.m[0] = 1.f / (aspect_ratio * tan_half_fovy);
	out.m[5] = 1.f / tan_half_fovy;
	out.m[10] = -(far_plane + near_plane) / (far_plane - near_plane);
	out.m[11] = -1.f;
	out.m[14] = -(2.f * far_plane * near_plane) / (far_plane - near_plane);

	return out;
}

Mat4 ortho(float left, float right, float bottom, float top, float near_plane, float far_plane)
{
	Mat4 out = IdentityMat;

	out.m[0] = 2.f / (right - left);
	out.m[5] = 2.f / (top - bottom);
	out.m[10] = -2.f / (far_plane - near_plane);
	out.m[12] = -(right + left) / (right - left);
	out.m[13] = -(top + bottom) / (top - bottom);
	out.m[14] = -(far_plane + near_plane) / (far_plane - near_plane);

	return out;
}

Mat4 look_at(Vec4 pos, Vec4 dir, Vec4 up)
{
	Vec4 f = sub_vec4(dir, pos);
	normaliz_vec4(&f);
	Vec4 s = cross_vec4(f, up);
	normaliz_vec4(&s);
	Vec4 u = cross_vec4(s, f);

	Mat4 out = IdentityMat;
	out.m[0] = s.x;
	out.m[4] = s.y;
	out.m[8] = s.z;

	out.m[1] = u.x;
	out.m[5] = u.y;
	out.m[9] = u.z;

	out.m[2] = -f.x;
	out.m[6] = -f.y;
	out.m[10] = -f.z;

	out.m[12] = -dot_vec4(s, pos);
	out.m[13] = -dot_vec4(u, pos);
	out.m[14] = dot_vec4(f, pos);

	return out;
}