#ifndef _ENTITY_H
#define _ENTITY_H

#include "lin_alg.h"
#include "global_decl.h"

// TODO: Add to entity's state
#define ENTITY_MOVEMENT_SPEED 150.f
#define ENTITY_DEFAULT_DAMAGE 10.f

enum EntityState
{
	EntityState_Setup,
	EntityState_Idle,
	EntityState_OnWindowResize,
	EntityState_Moving
};

typedef enum EntityType
{
	EntityType_None,
	EntityType_Square,
	EntityType_Circle,
	EntityType_Triangle
} EntityType;

typedef struct EntityDef
{
	enum EntityType   type;
	PathSegment*      path;
	int               path_idx;
	int               path_len;
	float             segment_percent;
	enum EntityState  state;
	int               drawable_handle;
	int               collidable2D_handle;
	int               initial_speed;
	int               speed;
	float             health;
	float             damage;
	int               alive;
	Vec2              direction;
} EntityDef;

int init_entity();
int add_entity(enum EntityType type, EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color);
int add_entity_path(EntityDef* dest, const PathDef* path, int path_len);

int find_enemy_with_collidable(EntityDef** dest, const Collidable2D* collidable);
int move_entity(EntityDef* dest, float pos_x, float pos_y);
int resize_entity(EntityDef* dest, float scale_x, float scale_y);


int entity_follow_path(EntityDef* entity);
void get_entities(EntityDef** dest);
int get_entities_num();

void entity_free_resources();

#endif // _ENTITY_H
