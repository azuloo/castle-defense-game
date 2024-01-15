#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "graphics.h"

enum EntityType
{
	Entity_Square,
	Entity_Circle,
	Entity_Triangle
};

enum EntityState
{
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
	int               path_len;
	enum EntityState  state;
} EntityDef;

int add_entity(enum EntityType type, EntityDef** dest);
int add_entity_path(EntityDef** dest, const PathSegment* path, int path_len);
int move_entity(EntityDef* entity, Vec3* new_pos);

#endif // _ENTITY_H
