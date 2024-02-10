#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "graphics.h"

int is_collided_AABB(DrawableDef* first, DrawableDef* second);
int physics_step();

#endif // _PHYSICS_H
