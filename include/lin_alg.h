#ifndef _LIN_ALG_H
#define _LIN_ALG_H

#include <stdint.h>
#include <math.h>
#include <stdbool.h>

/* row-major ordering */
typedef struct Mat4 {
	float m[16];
} Mat4;

typedef union Vec4 {
	float m[4];
	struct {
		float x, y, z, w;
	};
} Vec4;

typedef union Vec3 {
	float m[3];
	struct {
		float x, y, z;
	};
} Vec3;

typedef union Vec2 {
	float m[2];
	struct {
		float x, y;
	};
} Vec2;

typedef union Vec2i {
	int m[2];
	struct {
		int x, y;
	};
} Vec2i;

static const Vec4 XAxis = { {1, 0, 0, 0} };
static const Vec4 YAxis = { {0, 1, 0, 0} };
static const Vec4 ZAxis = { {0, 0, 1, 0} };
static const Vec4 InvXAxis = { {-1, 0, 0, 0} };
static const Vec4 InvYAxis = { {0, -1, 0, 0} };
static const Vec4 InvZAxis = { {0, 0, -1, 0} };

static const Vec2 Vec2_LEFT = { { -1.f, 0.f } };
static const Vec2 Vec2_RIGHT = { { 1.f, 0.f } };
static const Vec2 Vec2_UP = { { 0.f, 1.f } };
static const Vec2 Vec2_DOWN = { { 0.f, -1.f } };

static const Mat4 IdentityMat = { {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
} };

float math_min(float a, float b);
float math_max(float a, float b);
float math_clamp(float a, float min_val, float max_val);
float math_lerp(float a, float b, float f);

Vec2 vec2_multiply_by_scalar(float scalar, Vec2 v);
bool vec2_equals(Vec2 v1, Vec2 v2);

Mat4 multiply_mat4(const Mat4* m1, const Mat4* m2);
Vec4 mulmat_vec4(const Mat4* m, const Vec4* v);
void normaliz_vec4(Vec4* v);
void normaliz_vec3(Vec3* v);
Vec4 sub_vec4(Vec4 v1, Vec4 v2);
Vec4 add_vec4(Vec4 v1, Vec4 v2);
Vec3 sub_vec3(Vec3 v1, Vec3 v2);
Vec3 add_vec3(Vec3 v1, Vec3 v2);
Vec4 multipty_by_scalar(Vec4 v, float s);
float dot_vec4(Vec4 v1, Vec4 v2);
Vec4 cross_vec4(Vec4 v1, Vec4 v2);
void rotate(const Mat4* m, const Vec3* axis, float angle);
void rotate_x(const Mat4* m, float angle);
void rotate_y(const Mat4* m, float angle);
void rotate_z(const Mat4* m, float angle);
void scale(const Mat4* m, float x, float y, float z);
void translate(const Mat4* m, float x, float y, float z);
float degrees(float radians);
float radians(float degrees);

Mat4 perspective(float fovy, float aspect_ratio, float near_plane, float far_plane);
Mat4 ortho(float left, float right, float bottom, float top, float near_plane, float far_plane);

Mat4 look_at(Vec4 pos, Vec4 dir, Vec4 up);

#endif // _LIN_ALG_H
