#ifndef _DRAWABLE_OPS_H
#define _DRAWABLE_OPS_H

#include "global_decl.h"
#include "graphics.h"
#include "lin_alg.h"

#define COLOR_VEC_WHITE    (Vec4){ { 1.f, 1.f, 1.f, 1.f } }
#define COLOR_VEC_BLACK    (Vec4){ { 0.f, 0.f, 0.f, 1.f } }
#define COLOR_VEC_RED      (Vec4){ { 1.f, 0.f, 0.f, 1.f } }
#define COLOR_VEC_GREEN    (Vec4){ { 0.f, 1.f, 0.f, 1.f } }
#define COLOR_VEC_BLUE     (Vec4){ { 0.f, 0.f, 1.f, 1.f } }

// Translate - Scale
int drawable_transform_ts(DrawableDef* drawable, const char* uniform_name);

int get_quad_draw_buffer_data(DrawBufferData** dest);

int draw_quad(DrawableDef** dest, const Vec3* new_pos, const Vec3* new_scale, const Vec4* new_color, int render_layer);

#endif // _DRAWABLE_OPS_H
