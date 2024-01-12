#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "graphics.h"

typedef struct PathSegment
{
	Vec2 start;
	Vec2 end;
} PathSegment;

typedef struct EntityDef
{
	enum EntityType   type;
	PathSegment**     path;
	int               path_len;
	EntryCnf*         entry_cnf; // TODO: Use handles insted of pointers (realloc problem)
} EntityDef;

enum EntityType
{
	Square,
	Circle,
	Triangle
};

int add_entity(enum EntityType type, EntityDef** dest);
int add_entity_path(EntityDef** dest, const PathSegment* path, int path_len);
int move_entity(EntityDef* entity);

#endif // _ENTITY_H
