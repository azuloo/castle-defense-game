#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "global_decl.h"

// TODO: Add to entity's state
#define ENTITY_MOVEMENT_SPEED 250.f

// TODO: Enemy related, move out
enum EntityState
{
	Entity_Setup,
	Entity_Idle,
	Entity_Moving
};

// TODO: EnemyType; move out of here
typedef enum EntityType
{
	Entity_None,
	Entity_Square,
	Entity_Circle,
	Entity_Triangle,
	Entity_Castle
} EntityType;

typedef struct EntityDef
{
	enum EntityType   type;
	PathSegment**     path;
	int               path_idx;
	int               path_len;
	enum EntityState  state;
	int               drawable_handle;
	Collidable2D*     collidable2D;
} EntityDef;

int add_entity(enum EntityType type, EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color);
int add_entity_path(EntityDef* dest, const PathDef* path, int path_len);

int move_entity(EntityDef* dest, float pos_x, float pos_y);
int resize_entity(EntityDef* dest, float scale_x, float scale_y);

int find_entity_by_collidable2d(EntityDef** dest, const Collidable2D* collidable2D);

int entity_follow_path(EntityDef* entity);
void get_entities(EntityDef** dest);
int get_entities_num();

void entity_free_resources();

#endif // _ENTITY_H
