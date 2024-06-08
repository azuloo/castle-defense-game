#ifndef _HEALTH_BAR_H
#define _HEALTH_BAR_H

#include "global_decl.h"
#include "lin_alg.h"

#define HEALTH_BAR_DEFAULT_VALUE 1.f

typedef struct HealthBarDef
{
	Collidable2D* collision_box;
	float value;
	int handle;
	int drawable_handle;
} HealthBarDef;

int add_health_bar(int* dest_handle, const Vec3* pos, const Vec3* scale);
int get_health_bar(HealthBarDef** dest, int handle);
int change_health_bar_value(int handle, float amount);
void health_bar_free_resources();

#endif // _HEALTH_BAR_H
