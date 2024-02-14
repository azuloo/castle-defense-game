#ifndef _DRAWABLE_OPS_H
#define _DRAWABLE_OPS_H

#include "graphics.h"
#include "physics.h"

#define SQUARE_VERTICES_LEN   32
#define SQUARE_INDICES_LEN    6

// Translate - Scale
int drawable_transform_ts(DrawableDef* drawable, const char* uniform_name);

int get_square_draw_buffer_data(DrawBufferData** dest);

#endif // _DRAWABLE_OPS_H
