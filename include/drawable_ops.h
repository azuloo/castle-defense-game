#ifndef _DRAWABLE_OPS_H
#define _DRAWABLE_OPS_H

#include "graphics.h"
#include "physics.h"

#define SQUARE_VERTICES_LEN   32
#define SQUARE_INDICES_LEN    6

// Translate - Scale
int drawable_transform_ts(DrawableDef* drawable, const char* uniform_name);

int get_quad_draw_buffer_data(DrawBufferData** dest);

int draw_quad(DrawableDef** dest, const char* texture_path, int texture_type, const Vec3* new_pos, const Vec3* new_scale, const Vec4* new_color);

#endif // _DRAWABLE_OPS_H
