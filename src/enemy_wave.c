#include "enemy_wave.h"
#include "entity.h"
#include "utils.h"

#define MAX_ENEMY_WAVES 32 // Per-level limit.

EnemyWaveDef* s_EnemyWaves    = NULL;
int s_EnemyWavesAmount        = 0;
int s_EnemyWavesAllocated     = 0;

int get_enemy_wave(EnemyWaveDef** dest)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(s_EnemyWavesAllocated < s_EnemyWavesAmount, "[enemy_wave]: Maximum wave capacity is reached.");
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_EnemyWaves, "[enemy_wave]: Uninitialized enemy waves array. Did you forget to call init_enemy_waves()?");

	EnemyWaveDef* enemy_wave      = s_EnemyWaves + s_EnemyWavesAmount;
	enemy_wave->enemies           = NULL;
	enemy_wave->enemies_count     = 0;
	enemy_wave->spawn_interval    = 0.f;

	*dest = enemy_wave;
	s_EnemyWavesAmount++;

	return 0;
}

int init_enemy_waves(int amount)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(amount <= MAX_ENEMY_WAVES, "[enemy_wave]: Map count arg is too high.");
	EnemyWaveDef* enemy_waves = malloc(amount * sizeof *enemy_waves);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy_waves, "[enemy_wave]: Failed to allocate sufficient memory chunk for enemy waves.");

	s_EnemyWaves          = enemy_waves;
	s_EnemyWavesAmount    = amount;

	return 0;
}

void enemy_waves_free_resources()
{
	if (NULL != s_EnemyWaves)
	{
		// TODO: Free enemy_wave->enemies if needed
		free(s_EnemyWaves);
	}
}
