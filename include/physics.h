#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "entity.h"

typedef struct EntityDef EntityDef;

typedef struct PhysicsDef
{
	Vec3 pos;
	Vec3 scale;
	Vec3 rotation;

	int collidable;
} PhysicsDef;

int isCollidedAABB(EntityDef* first, EntityDef* second);
int physics_step();

#endif // _PHYSICS_H
