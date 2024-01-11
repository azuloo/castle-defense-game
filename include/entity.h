#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "graphics.h"

typedef struct EntityDef
{
	enum EntityType   type;
	Vec2**            path;
	EntryCnf*         entry_cnf;

} EntityDef;

enum EntityType
{
	Square,
	Circle,
	Triangle
};

int add_entity(enum EntityType type, EntityDef** dest);

#endif // _ENTITY_H
