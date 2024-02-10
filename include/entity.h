#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "graphics.h"
#include "physics.h"

// TODO: Add to entity's state
#define ENTITY_MOVEMENT_SPEED 250.f

typedef struct PhysicsDef PhysicsDef;

enum EntityType
{
	Entity_Square,
	Entity_Circle,
	Entity_Triangle,
	Entity_Castle
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

typedef struct EntityDef
{
	enum EntityType   type;
	PhysicsDef*       physics;
	PathSegment**     path;
	int               path_idx;
	int               path_len;
	enum EntityState  state;
	int               drawable_handle;
	int               collidable;
} EntityDef;

// TODO: Receive pos, scale and rotation as params here
int add_entity(enum EntityType type, EntityDef** dest);
int add_entity_path(EntityDef* dest, const PathSegment** path, int path_len);

int get_drawable_def(DrawableDef** dest, EntityDef* src);
int entity_follow_path(EntityDef* entity);
int get_entities(EntityDef** dest);
int get_entities_num();

void entity_free_resources();

#endif // _ENTITY_H
