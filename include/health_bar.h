#ifndef _HEALTH_BAR_H
#define _HEALTH_BAR_H

#include "global_decl.h"
#include "lin_alg.h"

#define HEALTH_BAR_DEFAULT_VALUE 100

typedef struct HealthBarDef
{
	Collidable2D* collision_box;
	int value;
	int handle;
	int drawable_handle;
} HealthBarDef;

int add_health_bar(const Vec3* pos, const Vec3* scale);
void health_bar_free_resources();

#endif // _HEALTH_BAR_H
