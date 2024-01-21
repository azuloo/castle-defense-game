#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "graphics.h"

// TODO: Add to entity's state
#define ENTITY_MOVEMENT_SPEED 400.f

enum EntityType
{
	Entity_Square,
	Entity_Circle,
	Entity_Triangle
};

enum EntityState
{
	Entity_Setup,
	Entity_Idle,
	Entity_Moving
};

typedef struct PathSegment
{
	Vec2 start;
	Vec2 end;
} PathSegment;

typedef struct TransformDef
{
	Vec3 pos;
	Vec3 scale;
	Vec3 rotation;
} TransformDef;

typedef struct EntityDef
{
	enum EntityType   type;
	TransformDef*     transform;
	PathSegment**     path;
	int               path_idx;
	int               path_len;
	enum EntityState  state;
	int               entry_handle;
} EntityDef;

// TODO: Receive pos, scale and rotation as params here
int add_entity(enum EntityType type, EntityDef** dest);
int add_entity_path(EntityDef* dest, const PathSegment* path, int path_len);
int entity_follow_path(EntityDef* entity);
void entity_free_resources();

#endif // _ENTITY_H
