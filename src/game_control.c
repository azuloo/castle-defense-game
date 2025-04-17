#include "game_control.h"
#include "enemy_wave.h"
#include "tower.h"
#include "player_controls.h"

void stop_game()
{
	set_enemy_wave_state(EnemyWaveState_Suspend);
	set_tower_state(TowerState_Suspend);
	enable_player_controls(0);
	disable_building_mode();
}
