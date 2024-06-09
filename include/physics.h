#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "lin_alg.h"
#include "global_decl.h"

#define MAX_COLLISION_HANDLES 16

enum PhysicsCollisionLayer
{
	CollisionLayer_None       = 0,
	CollisionLayer_Player     = (1U << 1),
	CollisionLayer_Enemy      = (1U << 2),
	CollisionLayer_Castle     = (1U << 3),
	CollisionLayer_Road       = (1U << 4),
	CollisionLayer_Tower      = (1U << 5),

	CollisionLayer_MAX        = (1U << 16)
};

enum CollisionState
{
	CollisionState_Collided     = (1U << 1),
	CollisionState_Uncollided   = (1U << 2),
};

typedef struct CollisionBox2D
{
	Vec3           position;
	Vec3           size;
	uint16_t       collision_layer;
	uint16_t       collision_mask;
#if DRAW_COLLISION_BOX_BOUNDS
	int            DEBUG_draw_bounds;
	DrawableDef*   DEBUG_bounds_drawable;
#endif // DRAW_COLLISION_BOX_BOUNDS

} CollisionBox2D;

typedef struct Collidable2D
{
	int               handle;
	CollisionBox2D    collision_box;
	int               collision_handles[MAX_COLLISION_HANDLES];
	int               collisions_detected;
} Collidable2D;

typedef void (*PhysicsCollisionEventCbPtr)(Collidable2D* first, Collidable2D* second);

void physics_add_collision_begind_cb(PhysicsCollisionEventCbPtr cb);
void physics_add_collision_end_cb(PhysicsCollisionEventCbPtr cb);

// ! Allocates memory on heap !
int add_collidable2D(int* handle_dest, const Vec3* initial_pos, const Vec3* initial_size);
void add_collision_layer2D(CollisionBox2D* collision_box, uint16_t layer);
void add_collision_mask2D(CollisionBox2D* collision_box, uint16_t mask);
int get_collidable2D(Collidable2D** dest, int handle);
int move_collision_box2D(CollisionBox2D* collision_box, float pos_x, float pos_y);
int resize_collision_box2D(CollisionBox2D* collision_box, float size_x, float size_y);

int is_collided_AABB(const CollisionBox2D* first, const CollisionBox2D* second);
int physics_step();
void physics_free_resources();

#endif // _PHYSICS_H
