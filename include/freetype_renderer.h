#ifndef _FREETYPE_RENDERER_H
#define _FREETYPE_RENDERER_H

#include "lin_alg.h"
#include "global_decl.h"

int render_text(const char* text, int text_len, int font_size, float x, float y, Vec3 color, int* drawable_handles_dest, int handles_count);
int rerender_text(const char* text, int text_len, int font_size, float x, float y, int* drawable_handles, int handles_count);

#endif // _FREETYPE_RENDERER_H
