#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "graphics.h"
#include "physics.h"

// TODO: Add to entity's state
#define ENTITY_MOVEMENT_SPEED 250.f

typedef enum EntityType
{
	Entity_None,
	Entity_Square,
	Entity_Circle,
	Entity_Triangle,
	Entity_Castle
} EntityType;

enum EntityState
{
	Entity_Setup,
	Entity_Idle,
	Entity_Moving
};

enum PhysicsCollisionLayer
{
	CollistionLayer_None        = 0,
	CollistionLayer_Player      = (1U << 1),
	CollistionLayer_Enemy       = (1U << 2),
	CollistionLayer_Castle      = (1U << 3),
};

typedef struct PathSegment
{
	Vec2 start;
	Vec2 end;
} PathSegment;

typedef struct EntityDef
{
	enum EntityType   type;
	PathSegment**     path;
	int               path_idx;
	int               path_len;
	enum EntityState  state;
	int               drawable_handle;
	CollisionBox2D*   collision_box;
} EntityDef;

int add_entity(enum EntityType type, EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color);
int add_entity_path(EntityDef* dest, const PathSegment** path, int path_len);
int add_entity_collision_box(EntityDef* dest);
int add_entity_collision_mask(EntityDef* dest, uint16_t mask);

int move_entity(EntityDef* dest, float pos_x, float pos_y);
int resize_entity(EntityDef* dest, float scale_x, float scale_y);

int get_drawable_def(DrawableDef** dest, EntityDef* src);
int entity_follow_path(EntityDef* entity);
int get_entities(EntityDef** dest);
int get_entities_num();

void entity_free_resources();

#endif // _ENTITY_H
