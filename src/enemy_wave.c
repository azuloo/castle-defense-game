#include "enemy_wave.h"
#include "entity.h"
#include "utils.h"
#include "map/map_mgr.h"
#include "global_defs.h"
#include "drawable_ops.h"

#include <time.h>

#define MAX_ENEMY_WAVES      32 // Per-level limit.
#define ENEMY_TYPES_AMOUNT   3

static EnemyWaveDef* s_EnemyWaves      = NULL;
static int s_EnemyWavesAmount          = 0;
static int s_CurrentWaveIdx            = 0;
static EntityType s_EnemyTypes[]       = {
	EntityType_Circle,
	EntityType_Square,
	EntityType_Triangle,
};

static int get_enemy_wave(EnemyWaveDef** dest)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(s_CurrentWaveIdx < s_EnemyWavesAmount, "[enemy_wave]: Maximum wave capacity is reached.");
	EnemyWaveDef* enemy_wave = s_EnemyWaves + s_CurrentWaveIdx;
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy_wave, "[enemy_wave]: Enemy wave is not allocated.");

	*dest = enemy_wave;

	return 0;
}

static int reset_data()
{
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_EnemyWaves, "[enemy_wave]: Enemy waves data has not been initiated.");
	for (int i = 0; i < s_EnemyWavesAmount; i++)
	{
		EnemyWaveDef* enemy_wave          = s_EnemyWaves + i;
		enemy_wave->enemies               = NULL;
		enemy_wave->enemies_left          = 0;
		enemy_wave->spawned_count         = 0;
		enemy_wave->init_delay_left       = 0.f;
		enemy_wave->spawn_interval_left   = 0.f;
		enemy_wave->state                 = EnemyWaveState_Init;
		enemy_wave->cnf                   = (EnemyWaveCnf){ EnemyWaveType_Random, 0, 0.f, 0.f };
	}
	
	return 0;
}

static int fill_enemies_data()
{
	EnemyWaveDef* enemy_wave = NULL;
	int get_enemy_wave_res = get_enemy_wave(&enemy_wave);
	CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != get_enemy_wave_res, "[enemy_wave]: Failed to get current enemy wave.");

	EntityDef* enemies = malloc(enemy_wave->cnf.enemies_amount * sizeof *enemies);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemies, "[enemy_wave]: Failed to allocate sufficient chunk of memory for enemies.");

	enemy_wave->enemies_left    = enemy_wave->cnf.enemies_amount;
	enemy_wave->enemies         = enemies;

	// TODO: Check for errors
	const PathDef* path    = map_mgr_get_path();
	int path_len           = map_mgr_get_path_len();
	Vec2 path_start        = map_mgr_get_path_start();

	Vec3 enemy_pos     = { { path_start.x, path_start.y, Z_DEPTH_INITIAL_ENTITY } };
	Vec3 enemy_scale   = { { 35.f, 35.f, 1.f } };
	Vec4 enemy_color   = COLOR_VEC_GREEN;

	EnemyWaveCnf* wave_cnf         = &enemy_wave->cnf;
	EntityDef* enemy               = NULL;
	DrawableDef* enemy_drawable    = NULL;

	for (int i = 0; i < wave_cnf->enemies_amount; i++)
	{
		enemy = NULL;

		switch (wave_cnf->type)
		{
		case EnemyWaveType_Random:
		{
			int enemy_idx = rand() % ENEMY_TYPES_AMOUNT;
			add_entity(s_EnemyTypes[enemy_idx], &enemy, &enemy_pos, &enemy_scale, &enemy_color);
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy, "[enemy_wave]: Failed to create an enemy.");
			add_entity_path(enemy, path, path_len);

			get_drawable_def(&enemy_drawable, enemy->drawable_handle);
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy_drawable, "[enemy_wave]: Failed to get drawable for this enemy.");
			enemy_drawable->visible = 0;
			// TODO: Add collision layer etc.

			EntityDef* enemy_slot = enemy_wave->enemies + i;
			memcpy(enemy_slot, enemy, sizeof(EntityDef));
		}
		break;

		case EnemyWaveType_Manual:
		{
			// TODO: Implement
		}
		break;

		case EnemyWaveType_Boss:
		{
			// TODO: Implement
		}
		break;

		default:
			break;
		}
	}

	return 0;
}

void advance_enemy_wave()
{
	s_CurrentWaveIdx++;
}

int init_enemy_waves(int amount)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(amount <= MAX_ENEMY_WAVES, "[enemy_wave]: Map count arg is too high.");
	EnemyWaveDef* enemy_waves = malloc(amount * sizeof *enemy_waves);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy_waves, "[enemy_wave]: Failed to allocate sufficient memory chunk for enemy waves.");

	s_EnemyWaves = enemy_waves;
	s_EnemyWavesAmount = amount;

	EnemyWaveCnf* enemy_waves_cnfs = malloc(amount * sizeof * enemy_waves_cnfs);
	if (NULL == enemy_waves_cnfs)
	{
		free(s_EnemyWaves);
		CHECK_EXPR_FAIL_RET_TERMINATE(false, "[enemy_wave]: Failed to allocate sufficient memory chunk for enemy waves cnfs.");
	}

	reset_data();
	srand(time(NULL));

	return 0;
}

int set_enemy_waves_cnf(EnemyWaveCnf* cnf, int amount)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(amount == s_EnemyWavesAmount, "[enemy_wave]: Enemy waves cnf amount should be less than enemy waves amount.");
	for (int i = 0; i < amount; i++)
	{
		// TODO: Check NULL
		EnemyWaveDef* enemy_wave = s_EnemyWaves + i;
		EnemyWaveCnf* enemy_wave_cnf_src = cnf + i;

		// TODO: Check failed op?
		memcpy(&enemy_wave->cnf, enemy_wave_cnf_src, sizeof(EnemyWaveCnf));
		enemy_wave->state = EnemyWaveState_Init;
	}

	return 0;
}

int enemy_waves_spawn(float frameTime)
{
	EnemyWaveDef* enemy_wave = NULL;
	int get_enemy_wave_res = get_enemy_wave(&enemy_wave);
	CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != get_enemy_wave_res, "[enemy_wave]: Failed to fetch enemy wave def.");

	switch (enemy_wave->state)
	{
	case EnemyWaveState_Init:
	{
		int fill_enemies_data_res = fill_enemies_data();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != fill_enemies_data_res, "[enemy_wave]: Failed to fill enemies data.");
		enemy_wave->init_delay_left = enemy_wave->cnf.init_delay;
		enemy_wave->spawn_interval_left = enemy_wave->cnf.spawn_interval;
		enemy_wave->state = EnemyWaveState_InitialDelay;
	}
	break;

	case EnemyWaveState_InitialDelay:
	{
		enemy_wave->init_delay_left -= frameTime;
		if (enemy_wave->init_delay_left <= 0.f)
		{
			enemy_wave->init_delay_left = enemy_wave->cnf.init_delay;
			enemy_wave->state = EnemyWaveState_Spawn;
		}
	}
	break;

	case EnemyWaveState_InterDelay:
	{
		enemy_wave->spawn_interval_left -= frameTime;
		if (enemy_wave->spawn_interval_left <= 0.f)
		{
			enemy_wave->spawn_interval_left = enemy_wave->cnf.spawn_interval;
			enemy_wave->state = EnemyWaveState_Spawn;
		}
	}
	break;

	case EnemyWaveState_Spawn:
	{
		if (enemy_wave->spawned_count >= enemy_wave->enemies_left)
		{
			enemy_wave->state = EnemyWaveState_Finished;
			break;
		}

		EntityDef* enemy = enemy_wave->enemies + enemy_wave->spawned_count;
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy, "[enemy_wave]: Cannot spawn enemy: empty EntityDef.");
		DrawableDef* enemy_drawable = NULL;
		get_drawable_def(&enemy_drawable, enemy->drawable_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy, "[enemy_wave]: Cannot spawn enemy: empty DrawableDef.");

		enemy_drawable->visible = 1;
		enemy_wave->spawned_count += 1;
		enemy_wave->state = EnemyWaveState_InterDelay;
	}
	break;

	case EnemyWaveState_Finished:
	{
	}
	break;

	default:
	{
		CHECK_EXPR_FAIL_RET_TERMINATE(false, "[enemy_wave]: Unrecognized state.");
	}
	break;
	}

	for (int i = 0; i < enemy_wave->spawned_count; i++)
	{
		EntityDef* enemy = enemy_wave->enemies + i;
		// TODO: List idx in error message?
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != enemy, "[enemy_wave]: No enemy found with this idx.");
		entity_follow_path(enemy);
	}
}

void reset_enemy_waves()
{
	reset_data();
}

void enemy_waves_free_resources()
{
	if (NULL != s_EnemyWaves)
	{
		for (int i = 0; i < s_EnemyWavesAmount; i++)
		{
			EnemyWaveDef* enemy_wave = s_EnemyWaves + i;
			if (NULL != enemy_wave && NULL != enemy_wave->enemies)
			{
				free(enemy_wave->enemies);
			}
		}

		free(s_EnemyWaves);
	}
}