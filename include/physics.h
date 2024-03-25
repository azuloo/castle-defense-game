#ifndef _PHYSICS_H
#define _PHYSICS_H

typedef struct EntityDef EntityDef;

typedef struct
{
	Vec3 position;
	Vec3 size;
#if DEBUG
	int            DEBUG_draw_bounds;
	DrawableDef*   DEBUG_bounds_drawable;
#endif // DEBUG

} CollisionBox2D;

typedef void (*PhysicsEntitiesCollidedCb)(EntityDef* first, EntityDef* second);

void physics_bind_entities_collided_cb(PhysicsEntitiesCollidedCb cb);

int is_collided_AABB(EntityDef* first, EntityDef* second);
int physics_step();

#endif // _PHYSICS_H
