#ifndef _ENTITY_H
#define _ENTITY_H

enum EntityType
{
	Square,
	Circle,
	Triangle
};

int add_entity(enum EntityType type);

#endif // _ENTITY_H
