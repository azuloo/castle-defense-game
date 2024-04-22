#ifndef _ENEMY_WAVE_H
#define _ENEMY_WAVE_H

#include "global_decl.h"

typedef enum EnemyWaveState
{
	EnemyWaveState_Init,
	EnemyWaveState_InitialDelay,
	EnemyWaveState_Spawn,
	EnemyWaveState_InterDelay,
	EnemyWaveState_Finished,

} EnemyWaveState;

typedef enum EnemyWaveType
{
	EnemyWaveType_Random,
	EnemyWaveType_Manual,
	EnemyWaveType_Boss,

} EnemyWaveType;

typedef struct EnemyWaveCnf
{
	EnemyWaveType   type;
	int             enemies_amount;
	float           init_delay;
	float           spawn_interval;

} EnemyWaveCnf;

typedef struct EnemyWaveDef
{
	int             state;
	int             enemies_left;
	int             spawned_count;
	float           init_delay_left;
	float           spawn_interval_left;
	EnemyWaveCnf    cnf;
	EntityDef*      enemies;
} EnemyWaveDef;

int get_enemy_wave(EnemyWaveDef** dest);
void advance_enemy_wave();
int init_enemy_waves(int amount);
int set_enemy_waves_cnf(EnemyWaveCnf* cnf, int amount);
int enemy_waves_spawn(float frameTime);
void reset_enemy_waves();
void enemy_waves_free_resources();

#endif // _ENEMY_WAVE_H
