#ifndef _GRAPHICS_DEFS_H
#define _GRAPHICS_DEFS_H

extern float wWidth;
extern float wHeight;

#include "lin_alg.h"

#define WINDOW_DEFAULT_RES_W 1800
#define WINDOW_DEFAULT_RES_H 920
#define WINDOW_DEFUALT_NAME "Application"

#define COMMON_ORTHO_MAT ortho(0.f, wWidth, 0.f, wHeight, -1.f, 1.f)

#endif // _GRAPHICS_DEFS_H
