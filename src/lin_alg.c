#include "lin_alg.h"

static const double PI = 3.14159265358979323846;

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
	float invrt = 1.f / sqrt(sqr);
	v->m[0] *= invrt;
	v->m[1] *= invrt;
	v->m[2] *= invrt;
}

Vec4 sub(Vec4 v1, Vec4 v2)
{
	Vec4 out = { {0} };
	out.m[0] = v1.m[0] - v2.m[0];
	out.m[1] = v1.m[1] - v2.m[1];
	out.m[2] = v1.m[2] - v2.m[2];
	out.m[3] = v1.m[3] - v2.m[3];
	return out;
}

float dot(Vec4 v1, Vec4 v2)
{
	return v1.m[0] * v2.m[0] + v1.m[1] * v2.m[1] + v1.m[2] * v2.m[2] + v1.m[3] * v2.m[3];
}

Vec4 cross(Vec4 v1, Vec4 v2)
{
	Vec4 out = { {0} };
	out.m[0] = v1.m[1] * v2.m[2] - v1.m[2] * v2.m[1];
	out.m[1] = v1.m[2] * v2.m[0] - v1.m[0] * v2.m[2];
	out.m[2] = v1.m[0] * v2.m[1] - v1.m[1] * v2.m[0];
	return out;
}

void rotate(const Vec3* axis, const Mat4* m, float angle)
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

	memcpy(m->m, multiply_mat4(m, &rotation).m, sizeof(m->m));
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

	memcpy(m->m, multiply_mat4(m, &rotation).m, sizeof(m->m));
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

	memcpy(m->m, multiply_mat4(m, &rotation).m, sizeof(m->m));
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

	memcpy(m->m, multiply_mat4(m, &rotation).m, sizeof(m->m));
}

void scale(const Mat4* m, float x, float y, float z)
{
	Mat4 scale = IdentityMat;

	scale.m[0] = x;
	scale.m[5] = y;
	scale.m[10] = z;

	memcpy(m->m, multiply_mat4(m, &scale).m, sizeof(m->m));
}

void translate(const Mat4* m, float x, float y, float z)
{
	Mat4 translation = IdentityMat;

	translation.m[12] = x;
	translation.m[13] = y;
	translation.m[14] = z;

	memcpy(m->m, multiply_mat4(m, &translation).m, sizeof(m->m));
}

float degrees(float radians)
{
	return radians * (180.f / PI);
}

float radians(float degrees)
{
	return degrees * (PI / 180.f);
}

Mat4 perspective(float fovy, float aspect_ratio, float near_plane, float far_plane)
{
	Mat4 out = { { 0 } };

	const float
		y_scale = (float)(1 / cos(fovy * PI / 360)),
		x_scale = y_scale / aspect_ratio,
		frustum_length = far_plane - near_plane;

	out.m[0] = x_scale;
	out.m[5] = y_scale;
	out.m[10] = -((far_plane + near_plane) / frustum_length);
	out.m[11] = -1;
	out.m[14] = -((2 * near_plane * far_plane) / frustum_length);

	return out;
}

Mat4 orthogonal(float left, float right, float bottom, float top)
{
	Mat4 out = IdentityMat;
	out.m[0] = 2 / (right - left);
	out.m[5] = 2 / (top - bottom);
	out.m[10] = -1;
	out.m[12] = -(right + left) / (right - left);
	out.m[13] = -(top + bottom) / (top - bottom);

	return out;
}

Mat4 look_at(Vec4 pos, Vec4 dir)
{
	Vec4 z = sub(pos, dir);
	normaliz_vec4(&z);
	Vec4 y = { {0.f, 1.f, 0.f, 0.f} };
	Vec4 x = cross(y, z);
	y = cross(z, x);

	normaliz_vec4(&x);
	normaliz_vec4(&y);

	Mat4 out = IdentityMat;
	out.m[0] = x.x;
	out.m[4] = x.y;
	out.m[8] = x.z;

	out.m[1] = y.x;
	out.m[5] = y.y;
	out.m[9] = y.z;

	out.m[2] = z.x;
	out.m[6] = z.y;
	out.m[10] = z.z;

	out.m[12] = -dot(x, pos);
	out.m[13] = -dot(y, pos);
	out.m[14] = -dot(z, pos);
	return out;
}