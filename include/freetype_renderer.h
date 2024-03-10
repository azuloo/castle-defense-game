#ifndef _FREETYPE_RENDERER_H
#define _FREETYPE_RENDERER_H

#include "lin_alg.h"
#include <stdlib.h>

int check_ft_buffers_created();
#define ASSERT_FT_BUFFERS_CREATED assert( check_ft_buffers_created() == 1 );

int ft_renderer_init();
int render_text(const char* text, float x, float y, Vec3 color);

#endif // _FREETYPE_RENDERER_H
