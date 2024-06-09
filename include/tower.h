#ifndef _TOWER_H
#define _TOWER_H

#include "global_decl.h"

#define DEFAULT_TOWER_ATTACK_POWER 10.f
#define DEFAULT_TOWER_PROJECTILE_SPEED 400.f

typedef struct TowerDef
{
	int handle;
	int drawable_handle;
	int collidable2D_handle;
	int collidable2D_detect_handle;
	float attack_power;
	float projectile_speed;
} TowerDef;

typedef enum TowerTypes
{
	TowerType_First,
	TowerType_Second,
	TowerType_Third,

	TowerType_Count
} TowerTypes;

int init_towers();
int resize_towers();
int place_new_tower_at_cursor();
int add_tower(int* handle_dest);
int get_tower(TowerDef** dest, int tower_handle);
void set_current_tower_preset_idx(int idx);
int create_build_tower_presets();
int find_tower_with_collidable(TowerDef** dest, const Collidable2D* collidable);
int on_select_tower_preset_pressed();
int on_tower_building_mode_enabled();

#endif // _TOWER_H
