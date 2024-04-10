#ifndef _PHYSICS_H
#define _PHYSICS_H

typedef struct EntityDef EntityDef;

typedef struct
{
	Vec3        position;
	Vec3        size;
	uint16_t    collision_layer;
	uint16_t    collision_mask;
#if DEBUG
	int            DEBUG_draw_bounds;
	DrawableDef*   DEBUG_bounds_drawable;
#endif // DEBUG

} CollisionBox2D;

enum PhysicsCollisionLayer
{
	CollistionLayer_None = 0,
	CollistionLayer_Player = (1U << 1),
	CollistionLayer_Enemy = (1U << 2),
	CollistionLayer_Castle = (1U << 3),
	CollistionLayer_Road = (1U << 4),

	CollistionLayer_MAX = (1U << 16)
};

typedef void (*PhysicsEntitiesCollidedCb)(EntityDef* first, EntityDef* second);

void physics_bind_entities_collided_cb(PhysicsEntitiesCollidedCb cb);
// ! Allocates memory on heap !
int add_collision_box2D(CollisionBox2D** dest, const Vec3* initial_pos, const Vec3* initial_size);
void add_collision_layer2D(CollisionBox2D* collision_box, uint16_t layer);
void add_collision_mask2D(CollisionBox2D* collision_box, uint16_t mask);
int move_collision_box2D(CollisionBox2D* collision_box, float pos_x, float pos_y);
int resize_collision_box2D(CollisionBox2D* collision_box, float size_x, float size_y);

int is_collided_AABB(EntityDef* first, EntityDef* second);
int physics_step();

#endif // _PHYSICS_H
