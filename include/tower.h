#ifndef _TOWER_H
#define _TOWER_H

#include "global_decl.h"
#include <stdbool.h>

#define DEFAULT_TOWER_ATTACK_POWER 25.f
#define DEFAULT_TOWER_PROJECTILE_SPEED 900.f
#define TOWER_FIRE_DELAY 2.f
#define PROJECTILES_PER_TOWER 3

typedef enum TowerTypes
{
	TowerType_First,
	TowerType_Second,
	TowerType_Third,

	TowerType_Count
} TowerTypes;

typedef enum TowerState
{
	TowerState_Idle,
	TowerState_FireDelay,
	TowerState_SpawnProjectile
} TowerState;

typedef enum ProjectileState
{
	ProjectileState_Init,
	ProjectileState_PostInit,
	ProjectileState_Moving,
	ProjectileState_Hit
} ProjectileState;

typedef struct ProjectileDef
{
	int drawable_handle;
	int collidable2D_handle;
	float projectile_speed;
	float damage_on_hit;
	int state;
	EntityDef* target;
	bool alive;
} ProjectileDef;

typedef struct TowerDef
{
	int handle;
	int drawable_handle;
	int collidable2D_handle;
	int collidable2D_detect_handle;
	int state;
	float attack_power;
	float fire_delay;
	bool spawned;
	ProjectileDef projectiles[PROJECTILES_PER_TOWER];
	List* targets;
} TowerDef;

int init_towers();
int update_towers(float dt);
int resize_towers();
int place_new_tower_at_cursor();
int add_tower(int* handle_dest);
int get_tower(TowerDef** dest, int tower_handle);
void set_current_tower_preset_idx(int idx);
int create_build_tower_presets();
int find_tower_with_collidable(TowerDef** dest, const Collidable2D* collidable);
int on_select_tower_preset_pressed();
int on_tower_building_mode_enabled();
int find_projectile_with_collidable(ProjectileDef** dest, const Collidable2D* collidalbe);

#endif // _TOWER_H
