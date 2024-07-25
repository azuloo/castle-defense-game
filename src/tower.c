#include "tower.h"
#include "graphics.h"
#include "physics.h"
#include "utils.h"
#include "drawable_ops.h"
#include "global_defs.h"
#include "enemy_wave.h" // TODO: Potentially, to be removed
#include "entity.h" // TODO: Potentially, to be removed
#include "container/list.h"

#define MAX_TOWER_CAPACITY 64
#define MOVING_PROJECTILE_CAPACITY 64
#define DEFAULT_PROJECTILE_POS_X 0.f
#define DEFAULT_PROJECTILE_POS_Y 0.f

extern int wWidth;
extern int wHeight;
extern int xWOffset;
extern int yWOffset;
extern double s_CursorXPos;
extern double s_CursorYPos;

extern float get_window_scale_x();
extern float get_window_scale_y();

static TowerDef s_TowersData[MAX_TOWER_CAPACITY];
static int s_TowersCount = 0;
static int s_SpawnedTowers[MAX_TOWER_CAPACITY];
static int s_SpawnedTowersCount = 0;

static int s_CurrentTowerIdx = 0;
static TowerDef* tower_presets[TowerType_Count];

static ProjectileDef* s_MovingProjectiles[MOVING_PROJECTILE_CAPACITY];

static const char* first_texture_path = "/res/static/textures/triangle.png";
static const char* second_texture_path = "/res/static/textures/square.png";
static const char* third_texture_path = "/res/static/textures/circle.png";

static const int tower_texture_params[DEFAULT_TEXTURE_PARAMS_COUNT] = {
	GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER,
	GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER,
	GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR,
	GL_TEXTURE_MAG_FILTER, GL_LINEAR
};

static void init_tower_projectiles(TowerDef* tower, float speed, float damage)
{
	for (int i = 0; i < PROJECTILES_PER_TOWER; i++)
	{
		ProjectileDef* projectile          = &tower->projectiles[i];
		projectile->drawable_handle        = -1;
		projectile->collidable2D_handle    = -1;
		projectile->projectile_speed       = speed;
		projectile->damage_on_hit          = damage;
		projectile->state                  = ProjectileState_Init;
	}
}

static int create_tower_at(TowerDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color, const char* texture_path)
{
	int tower_handle = -1;
	add_tower(&tower_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(-1 != tower_handle, "[tower]: Failed to add a tower.");

	TowerDef* tower = NULL;
	get_tower(&tower, tower_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(-1 != tower_handle, "[tower]: Failed to fetch a tower.");

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, pos, scale, color, texture_path, TexType_RGBA, tower_texture_params, DEFAULT_TEXTURE_PARAMS_COUNT);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[tower]: Failed to create a first tower preset (empty quad drawable).");

	tower->drawable_handle = drawable->handle;

	*dest = tower;

	return 0;
}

static int find_tower_with_collidable(TowerDef** dest, const Collidable2D* collidable)
{
	for (int i = 0; i < s_TowersCount; i++)
	{
		const TowerDef* tower = s_TowersData + i;
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower, "[tower] The tower with this idx has not been initialized.");

		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, tower->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");

		if (collidable2D->handle == collidable->handle)
		{
			*dest = tower;
			break;
		}
	}

	return 0;
}

static int find_tower_by_detection_collidable(TowerDef** dest, const Collidable2D* collidable)
{
	for (int i = 0; i < s_TowersCount; i++)
	{
		const TowerDef* tower = s_TowersData + i;
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower, "[tower] The tower with this idx has not been initialized.");

		if (-1 == tower->collidable2D_detect_handle)
		{
			continue;
		}

		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, tower->collidable2D_detect_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");

		if (collidable2D->handle == collidable->handle)
		{
			*dest = tower;
			break;
		}
	}

	return 0;
}

static void process_tower_collidable(Collidable2D* first, Collidable2D* second)
{
	TowerDef* tower = NULL;

	if (first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&tower, first);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&tower, second);
	}

	if (NULL != tower)
	{
		if (first->collision_box.collision_layer & CollisionLayer_Tower)
		{
			if (second->collision_box.collision_layer & (CollisionLayer_Road | CollisionLayer_Castle | CollisionLayer_Tower))
			{
				DrawableDef* first_drawable = NULL;
				get_drawable_def(&first_drawable, tower->drawable_handle);

				Vec4 color_vec = COLOR_VEC_RED;

				add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
		else if (second->collision_box.collision_layer & CollisionLayer_Tower)
		{
			if (first->collision_box.collision_layer & (CollisionLayer_Road | CollisionLayer_Castle | CollisionLayer_Tower))
			{
				DrawableDef* second_drawable = NULL;
				get_drawable_def(&second_drawable, tower->drawable_handle);

				Vec4 color_vec = COLOR_VEC_RED;

				add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
	}
}

static void process_projectile_collidable(Collidable2D* first, Collidable2D* second)
{
	ProjectileDef* projectile = NULL;

	if (first->collision_box.collision_layer & CollisionLayer_Projectile)
	{
		find_projectile_with_collidable(&projectile, first);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Projectile)
	{
		find_projectile_with_collidable(&projectile, second);
	}

	if (NULL != projectile)
	{
		projectile->alive = false;
		projectile->state = ProjectileState_Hit;
	}
}

static void process_detection_collidable_begin_hook(Collidable2D* first, Collidable2D* second)
{
	EntityDef* enemy = NULL;
	TowerDef* tower = NULL;

	if (first->collision_box.collision_layer & CollisionLayer_Detection && second->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&enemy, second);
		CHECK_EXPR_FAIL_RET(NULL != enemy, "[tower]: Failed to find the enemy with collidable.");
		find_tower_by_detection_collidable(&tower, first);
		CHECK_EXPR_FAIL_RET(NULL != tower, "[tower]: Failed to find the tower with collidable.");
	}
	if (second->collision_box.collision_layer & CollisionLayer_Detection && first->collision_box.collision_layer & CollisionLayer_Enemy)
	{
		find_enemy_with_collidable(&enemy, first);
		CHECK_EXPR_FAIL_RET(NULL != enemy, "[tower]: Failed to find the enemy with collidable.");
		find_tower_by_detection_collidable(&tower, second);
		CHECK_EXPR_FAIL_RET(NULL != tower, "[tower]: Failed to find the tower with collidable.");
	}

	if (NULL != tower && NULL != enemy)
	{
		add_to_list(tower->targets, (void*)enemy, sizeof *enemy);
	}
}

static void process_detection_collidable_end_hook(Collidable2D* first, Collidable2D* second)
{
	
}

static void process_collision_begin_hook(Collidable2D* first, Collidable2D* second)
{
	process_tower_collidable(first, second);
	process_projectile_collidable(first, second);
	process_detection_collidable_begin_hook(first, second);
}

static void process_collision_end_hook(Collidable2D* first, Collidable2D* second)
{
	TowerDef* tower = NULL;

	if (first->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&tower, first);
	}
	else if (second->collision_box.collision_layer & CollisionLayer_Tower)
	{
		find_tower_with_collidable(&tower, second);
	}

	if (NULL != tower)
	{
		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, tower->collidable2D_handle);
		CHECK_EXPR_FAIL_RET(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");

		if (first->collision_box.collision_layer & CollisionLayer_Tower)
		{
			bool road_collision_pred = second->collision_box.collision_layer & CollisionLayer_Road && collidable2D->collisions_detected == 0;
			bool castle_collision_pred = second->collision_box.collision_layer & CollisionLayer_Castle && collidable2D->collisions_detected == 0;
			bool tower_collision_pred = second->collision_box.collision_layer & CollisionLayer_Tower && collidable2D->collisions_detected == 0;
			if (road_collision_pred || castle_collision_pred || tower_collision_pred)
			{
				DrawableDef* first_drawable = NULL;
				get_drawable_def(&first_drawable, tower->drawable_handle);

				Vec4 color_vec = COLOR_VEC_GREEN;

				add_uniform_vec4f(first_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
		else if (second->collision_box.collision_layer & CollisionLayer_Tower)
		{
			bool road_collision_pred = first->collision_box.collision_layer & CollisionLayer_Road && collidable2D->collisions_detected == 0;
			bool castle_collision_pred = first->collision_box.collision_layer & CollisionLayer_Castle && collidable2D->collisions_detected == 0;
			bool tower_collision_pred = first->collision_box.collision_layer & CollisionLayer_Tower && collidable2D->collisions_detected == 0;
			if (road_collision_pred || castle_collision_pred || tower_collision_pred)
			{
				DrawableDef* second_drawable = NULL;
				get_drawable_def(&second_drawable, tower->drawable_handle);

				Vec4 color_vec = COLOR_VEC_GREEN;

				add_uniform_vec4f(second_drawable->shader_prog, COMMON_COLOR_UNIFORM_NAME, &color_vec);
			}
		}
	}
}

static int get_enemy_to_attack(const TowerDef* tower, EntityDef** dest)
{
	if (tower->targets->length > 0)
	{
		ListNode* enemy_node = tower->targets->head;
		EntityDef* enemy = (EntityDef*)enemy_node->data;
		if (enemy != NULL && enemy->alive)
		{
			*dest = enemy;
		}
		else
		{
			for (int i = 0; i < tower->targets->length-1; i++)
			{
				enemy_node = enemy_node->next;
				enemy = (EntityDef*)enemy_node->data;
				if (enemy != NULL && enemy->alive)
				{
					*dest = enemy;
					break;
				}
			}
		}
	}

	return 0;
}

static int add_moving_projectile(const ProjectileDef* projectile)
{
	bool found = false;
	for (int i = 0; i < MOVING_PROJECTILE_CAPACITY; i++)
	{
		if (NULL == s_MovingProjectiles[i])
		{
			s_MovingProjectiles[i] = projectile;
			found = true;
			break;
		}
	}

	if (!found)
	{
		PRINT_ERR("[tower]: Failed to find available slot in moving projectiles arr.");
	}

	return 0;
}

static int removed_moving_projectile(const ProjectileDef* projectile)
{
	bool found = false;
	for (int i = 0; i < MOVING_PROJECTILE_CAPACITY; i++)
	{
		if (s_MovingProjectiles[i] == projectile)
		{
			s_MovingProjectiles[i] = NULL;
			found = true;
			break;
		}
	}

	if (!found)
	{
		PRINT_ERR("[tower]: Failed to find a projectile to remove.");
	}

	return 0;
}

int init_towers()
{
	physics_add_collision_begind_cb(&process_collision_begin_hook);
	physics_add_collision_end_cb(&process_collision_end_hook);

	create_build_tower_presets();

	for (int i = 0; i < MOVING_PROJECTILE_CAPACITY; i++)
	{
		s_MovingProjectiles[i] = NULL;
	}

	return 0;
}

int update_towers(float dt)
{
	for (int i = 0; i < s_SpawnedTowersCount; i++)
	{
		TowerDef* tower = s_TowersData + s_SpawnedTowers[i];
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower, "[tower]: The tower ptr is empty.");

		switch (tower->state)
		{
		case TowerState_Idle:
		{
			EntityDef* entity = NULL;
			get_enemy_to_attack(tower, &entity);
			if (NULL != entity)
			{
				tower->state = TowerState_SpawnProjectile;
			}
		} break;

		case TowerState_SpawnProjectile:
		{
			// TODO: Check for collision end here
			ProjectileDef* projectile = NULL;
			for (int j = 0; j < PROJECTILES_PER_TOWER; j++)
			{
				// TODO: Select the first free projectile
				projectile = &tower->projectiles[j];
				if (projectile->state == ProjectileState_Init)
				{
					break;
				}

				projectile = NULL;
			}

			if (NULL != projectile)
			{
				DrawableDef* tower_drawable = NULL;
				get_drawable_def(&tower_drawable, tower->drawable_handle);

				Vec3 projectile_scale = { { 10.f, 10.f, 1.f } };
				Vec4 projectile_color = COLOR_VEC_RED;

				Vec3 projectile_pos = tower_drawable->transform.translation;
				// TODO: Change to const
				projectile_pos.z = 0.89f;

				DrawableDef* projectile_drawable = NULL;

				if (-1 == projectile->drawable_handle)
				{
					draw_quad(&projectile_drawable, &projectile_pos, &projectile_scale, &projectile_color, first_texture_path, TexType_RGBA, tower_texture_params, DEFAULT_TEXTURE_PARAMS_COUNT);
					projectile->drawable_handle = projectile_drawable->handle;
				}
				else
				{
					get_drawable_def(&projectile_drawable, projectile->drawable_handle);
				}

				projectile->state = ProjectileState_Moving;
				projectile->alive = true;

				projectile_drawable->transform.translation = projectile_pos;
				projectile_drawable->transform.scale = projectile_scale;

				projectile_drawable->visible = 1;

				if (-1 == projectile->collidable2D_handle)
				{
					add_collidable2D(&projectile->collidable2D_handle, &projectile_drawable->transform.translation, &projectile_drawable->transform.scale);
				}
				
				Collidable2D* collidable2D = NULL;
				get_collidable2D(&collidable2D, projectile->collidable2D_handle);
				CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower]: Failed to fetch Collidable2D for a projectile.");

				add_collision_layer2D(&collidable2D->collision_box, CollisionLayer_Projectile);
				add_collision_mask2D(&collidable2D->collision_box, CollisionLayer_Enemy);

				drawable_transform_ts(projectile_drawable, COMMON_MODEL_UNIFORM_NAME);

				add_moving_projectile(projectile);

				tower->state = TowerState_FireDelay;
			}

		} break;

		case TowerState_FireDelay:
		{
			if (tower->fire_delay <= 0.f)
			{
				tower->state = TowerState_Idle;
				tower->fire_delay = TOWER_FIRE_DELAY;
			}
			else
			{
				tower->fire_delay -= dt;
			}

		} break;
		}

		ProjectileDef* projectile = NULL;
		for (int j = 0; j < PROJECTILES_PER_TOWER; j++)
		{
			projectile = &tower->projectiles[j];

			if (NULL == projectile)
			{
				continue;
			}

			switch (projectile->state)
			{
			case ProjectileState_Init:
			{
			} break;

			case ProjectileState_Moving:
			{
				EntityDef* enemy = NULL;
				get_enemy_to_attack(tower, &enemy);

				// TODO: Do we have to do something additional here?
				if (NULL == enemy)
				{
					projectile->state = ProjectileState_Hit;
					continue;
				}

				DrawableDef* entity_drawable = NULL;
				get_drawable_def(&entity_drawable, enemy->drawable_handle);

				Vec3 entity_pos = entity_drawable->transform.translation;

				DrawableDef* drawable = NULL;
				get_drawable_def(&drawable, projectile->drawable_handle);

				Vec3 tower_pos = drawable->transform.translation;

				Vec3 projectile_dir = sub_vec3(entity_pos, tower_pos);
				normaliz_vec3(&projectile_dir);

				float new_pos_x = drawable->transform.translation.x + projectile->projectile_speed * dt * projectile_dir.x;
				float new_pos_y = drawable->transform.translation.y + projectile->projectile_speed * dt * projectile_dir.y;

				// TODO: Change zdepth for const
				Vec3 new_projectile_pos = { { new_pos_x, new_pos_y, 0.87f } };

				drawable->transform.translation = new_projectile_pos;

				Collidable2D* collidable2D = NULL;
				get_collidable2D(&collidable2D, projectile->collidable2D_handle);
				CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower]: Failed to fetch Collidable2D for a projectile.");

				move_collision_box2D(&collidable2D->collision_box, new_pos_x, new_pos_y);

				drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

			} break;

			case ProjectileState_Hit:
			{
				DrawableDef* projectile_drawable = NULL;
				get_drawable_def(&projectile_drawable, projectile->drawable_handle);
				CHECK_EXPR_FAIL_RET_TERMINATE(NULL != projectile_drawable, "[tower]: Failed to find a drawable for the projectile.");

				projectile_drawable->visible = 0;
				projectile_drawable->transform.translation = (Vec3){ DEFAULT_PROJECTILE_POS_X, DEFAULT_PROJECTILE_POS_Y, projectile_drawable->transform.translation.z };

				Collidable2D* collidable2D = NULL;
				get_collidable2D(&collidable2D, projectile->collidable2D_handle);
				CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");

				add_collision_layer2D(&collidable2D->collision_box, CollisionLayer_None);
				add_collision_mask2D(&collidable2D->collision_box, CollisionLayer_None);

				move_collision_box2D(&collidable2D->collision_box, DEFAULT_PROJECTILE_POS_X, DEFAULT_PROJECTILE_POS_Y);

				drawable_transform_ts(projectile_drawable, COMMON_MODEL_UNIFORM_NAME);

				removed_moving_projectile(projectile);

				projectile->state = ProjectileState_Init;
			} break;

			}
		}
	}

	return 0;
}

int resize_towers()
{
	for (int i = 0; i < TowerType_Count; i++)
	{
		TowerDef* tower = tower_presets[i];
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower, "[tower]: A tower ptr has not been initialized.");
		DrawableDef* tower_drawable = NULL;

		get_drawable_def(&tower_drawable, tower->drawable_handle);

		if (NULL != tower_drawable)
		{
			float scaleX = get_window_scale_x();
			float scaleY = get_window_scale_y();
			float tower_scale_x = tower_drawable->init_transform.scale.x * scaleX;
			float tower_scale_y = tower_drawable->init_transform.scale.y * scaleY;

			// TODO: Can be replaced with common method in Physics
			tower_drawable->transform.scale.x = tower_scale_x;
			tower_drawable->transform.scale.y = tower_scale_y;

			if (-1 != tower->collidable2D_handle)
			{
				Collidable2D* collidable2D = NULL;
				get_collidable2D(&collidable2D, tower->collidable2D_handle);
				CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");

				resize_collision_box2D(&collidable2D->collision_box, tower_scale_x, tower_scale_y);
			}

			drawable_transform_ts(tower_drawable, COMMON_MODEL_UNIFORM_NAME);
		}
	}

	return 0;
}

int place_new_tower_at_cursor()
{
	TowerDef* tower_preset = tower_presets[s_CurrentTowerIdx];
	Collidable2D* collidable2D = NULL;
	get_collidable2D(&collidable2D, tower_preset->collidable2D_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");

	// Spawn a tower only if it's not colliding with anything.
	if (collidable2D->collisions_detected == 0)
	{
		TowerDef* tower = NULL;

		float tower_x_pos = (float)s_CursorXPos - xWOffset;
		float tower_y_pos = (float)wHeight - (float)s_CursorYPos + yWOffset;

		Vec3 pos = { { tower_x_pos, tower_y_pos, Z_DEPTH_INITIAL_ENTITY } };
		Vec4 color = COLOR_VEC_GREEN;

		DrawableDef* tower_preset_drawable = NULL;
		get_drawable_def(&tower_preset_drawable, tower_preset->drawable_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower_preset_drawable, "[tower]: Failed to get tower drawable.");

		switch (s_CurrentTowerIdx)
		{
		case TowerType_First:
		{
			create_tower_at(&tower, &pos, &tower_preset_drawable->transform.scale, &color, first_texture_path);
		}
		break;

		case TowerType_Second:
		{
			create_tower_at(&tower, &pos, &tower_preset_drawable->transform.scale, &color, second_texture_path);
		}
		break;

		case TowerType_Third:
		{
			create_tower_at(&tower, &pos, &tower_preset_drawable->transform.scale, &color, third_texture_path);
		}
		break;
		default:
		{
			PRINT_ERR("[tower]: Unknown tower type.");
		}
		break;
		}

		if (NULL != tower)
		{
			DrawableDef* drawable = NULL;
			get_drawable_def(&drawable, tower->drawable_handle);
			CHECK_EXPR_FAIL_RET_TERMINATE(drawable != NULL, "[tower]: Failed to fetch a tower drawable.");

			add_collidable2D(&tower->collidable2D_handle, &drawable->transform.translation, &drawable->transform.scale);
			CHECK_EXPR_FAIL_RET_TERMINATE(-1 != tower->collidable2D_handle, "[tower]: Failed to attach Collidable2D.");

			Vec3 detect_collidable_scale = multipty_by_scalar_vec3(drawable->transform.scale, 10);

			add_collidable2D(&tower->collidable2D_detect_handle, &drawable->transform.translation, &detect_collidable_scale);
			CHECK_EXPR_FAIL_RET_TERMINATE(-1 != tower->collidable2D_detect_handle, "[tower]: Failed to attach detection Collidable2D.");

			Collidable2D* collidable2D = NULL;
			get_collidable2D(&collidable2D, tower->collidable2D_handle);
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");
			add_collision_layer2D(&collidable2D->collision_box, CollisionLayer_Tower);

			Collidable2D* detection_collidable2D = NULL;
			get_collidable2D(&detection_collidable2D, tower->collidable2D_detect_handle);
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != detection_collidable2D, "[tower]: Failed to get detection Collidable2D for the tower.");

			add_collision_layer2D(&detection_collidable2D->collision_box, CollisionLayer_Detection);
			add_collision_mask2D(&detection_collidable2D->collision_box, CollisionLayer_Enemy);

			tower->spawned = true;
			s_SpawnedTowers[s_SpawnedTowersCount] = tower->handle;
			s_SpawnedTowersCount++;

			tower_preset_drawable->visible = 0;
		}

		return 1;
	}

	return 0;
}

int add_tower(int* handle_dest)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(s_TowersCount < MAX_TOWER_CAPACITY, "[tower]: Max tower capacity has been reached.");

	TowerDef* tower                      = s_TowersData + s_TowersCount;
	tower->handle                        = s_TowersCount;
	tower->drawable_handle               = -1;
	tower->collidable2D_handle           = -1;
	tower->collidable2D_detect_handle    = -1;
	tower->state                         = TowerState_Idle;
	tower->attack_power                  = DEFAULT_TOWER_ATTACK_POWER;
	tower->fire_delay                    = TOWER_FIRE_DELAY;
	tower->spawned                       = false;
	tower->targets                       = NULL;

	init_tower_projectiles(tower, DEFAULT_TOWER_PROJECTILE_SPEED, tower->attack_power);
	create_list(&tower->targets);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower->targets, "[tower]: Failed to create targets list for the tower.");
	
	*handle_dest = tower->handle;

	s_TowersCount++;

	return 0;
}

int get_tower(TowerDef** dest, int tower_handle)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(tower_handle >= 0 && tower_handle < s_TowersCount, "[tower]: Tower handle is out of bounds.");

	TowerDef* tower = s_TowersData + tower_handle;
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower, "[tower]: The requested tower is non-existent.");

	*dest = tower;

	return 0;
}

void set_current_tower_preset_idx(int idx)
{
	CHECK_EXPR_FAIL_RET(idx >= 0 && idx < TowerType_Count, "[tower]: Supplied tower index is out of bounds.");
	s_CurrentTowerIdx = idx;
}

int create_build_tower_presets()
{
	TowerDef* tower_preset = NULL;
	DrawableDef* tower_drawable = NULL;

	Vec4 color = COLOR_VEC_GREEN;

	for (int tower_type = 0; tower_type < TowerType_Count; tower_type++)
	{
		switch (tower_type)
		{
		case TowerType_First:
		{
			Vec3 pos = { { 700.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_TOWER } };
			Vec3 scale = { { (float)wHeight * 0.03f, (float)wHeight * 0.03f, 1.f } };

			create_tower_at(&tower_preset, &pos, &scale, &color, first_texture_path);
		} break;

		case TowerType_Second:
		{
			Vec3 pos = { { 900.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
			Vec3 scale = { { (float)wHeight * 0.03f, (float)wHeight * 0.03f, 1.f } };

			create_tower_at(&tower_preset, &pos, &scale, &color, second_texture_path);
		} break;

		case TowerType_Third:
		{
			Vec3 pos = { { 1100.f, wHeight / 2.f + 200.f, Z_DEPTH_INITIAL_ENTITY } };
			Vec3 scale = { { (float)wHeight * 0.03f, (float)wHeight * 0.03f, 1.f } };

			create_tower_at(&tower_preset, &pos, &scale, &color, third_texture_path);
		} break;

		default:
			break;
		}

		tower_presets[tower_type] = tower_preset;

		get_drawable_def(&tower_drawable, tower_preset->drawable_handle);
		tower_drawable->visible = 0;

		add_collidable2D(&tower_preset->collidable2D_handle, &tower_drawable->transform.translation, &tower_drawable->transform.scale);

		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, tower_preset->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");
		add_collision_layer2D(&collidable2D->collision_box, CollisionLayer_Tower);
		add_collision_mask2D(&collidable2D->collision_box, CollisionLayer_Road | CollisionLayer_Castle | CollisionLayer_Tower);

		tower_preset = NULL;
	}

	return 0;
}

int on_select_tower_preset_pressed()
{
	DrawableDef* tower_drawable = NULL;

	for (int tower_type = 0; tower_type < TowerType_Count; tower_type++)
	{
		get_drawable_def(&tower_drawable, tower_presets[tower_type]->drawable_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower_drawable, "[tower] Failed to fetch a drawable for the tower.");
		tower_drawable->visible = 0;
	}

	get_drawable_def(&tower_drawable, tower_presets[s_CurrentTowerIdx]->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower_drawable, "[tower] Failed to fetch a drawable for the tower.");
	tower_drawable->visible = 1;

	return 0;
}

int on_tower_building_mode_enabled()
{
	TowerDef* tower = tower_presets[s_CurrentTowerIdx];
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != tower, "[tower]: A tower preset has not been initialized.");
	DrawableDef* tower_drawable = NULL;

	get_drawable_def(&tower_drawable, tower->drawable_handle);

	if (NULL != tower_drawable)
	{
		float tower_x_pos = (float)s_CursorXPos - xWOffset;
		float tower_y_pos = (float)wHeight - (float)s_CursorYPos + yWOffset;
		
		tower_drawable->transform.translation.x = tower_x_pos;
		tower_drawable->transform.translation.y = tower_y_pos;

		if (-1 != tower->collidable2D_handle)
		{
			Collidable2D* collidable2D = NULL;
			get_collidable2D(&collidable2D, tower->collidable2D_handle);
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[tower] Failed to fetch Collidable2D for the tower.");

			move_collision_box2D(&collidable2D->collision_box, tower_x_pos, tower_y_pos);
		}

		drawable_transform_ts(tower_drawable, COMMON_MODEL_UNIFORM_NAME);
	}

	return 0;
}

int find_projectile_with_collidable(ProjectileDef** dest, const Collidable2D* collidalbe)
{
	for (int i = 0; i < MOVING_PROJECTILE_CAPACITY; i++)
	{
		ProjectileDef* projectile = s_MovingProjectiles[i];

		if (NULL != projectile && projectile->collidable2D_handle == collidalbe->handle)
		{
			*dest = projectile;
			break;
		}
	}

	return 0;
}
