#ifndef _ENEMY_WAVE_H
#define _ENEMY_WAVE_H

#include "global_decl.h"

typedef struct EnemyWaveDef
{
	float          spawn_interval;
	int            enemies_count;
	EntityDef**    enemies;
} EnemyWaveDef;

int get_enemy_wave(EnemyWaveDef** dest);
int init_enemy_waves(int amount);
void enemy_waves_free_resources();

#endif // _ENEMY_WAVE_H
