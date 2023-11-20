#ifndef _LIN_ALG_H
#define _LIN_ALG_H

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

static const Vec4 XAxis = { {1, 0, 0, 0} };
static const Vec4 YAxis = { {0, 1, 0, 0} };
static const Vec4 ZAxis = { {0, 0, 1, 0} };
static const Vec4 InvXAxis = { {-1, 0, 0, 0} };
static const Vec4 InvYAxis = { {0, -1, 0, 0} };
static const Vec4 InvZAxis = { {0, 0, -1, 0} };

static const Mat4 IdentityMat = { {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
} };

Mat4 multiply_mat4(const Mat4* m1, const Mat4* m2);
Vec4 mulmat_vec4(const Mat4* m, const Vec4* v);
void normaliz_vec4(Vec4* v);
float dot(Vec4 v1, Vec4 v2);
Vec4 cross(Vec4 v1, Vec4 v2);
void rotate_x(const Mat4* m, float angle);
void rotate_y(const Mat4* m, float angle);
void rotate_z(const Mat4* m, float angle);
void scale(const Mat4* m, float x, float y, float z);
void translate(const Mat4* m, float x, float y, float z);

Mat4 perspective(float fovy, float aspect_ratio, float near_plane, float far_plane);
Mat4 orthogonal(float left, float right, float bottom, float top);

Mat4 look_at(Vec4 pos, Vec4 dir);

#endif // _LIN_ALG_H
