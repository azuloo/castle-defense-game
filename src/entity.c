#include "entity.h"
#include "file_reader.h"
#include "utils.h"
#include "obj_registry.h"
#include "global_defs.h"
#include "graphics_defs.h"
#include "drawable_ops.h"
#include "file_reader.h"
#include "map/map_mgr.h"

#include <stdlib.h>
#include <stdbool.h>

#define ENTITY_SHADER_COLOR_UNIFORM_NAME "UColor"

extern int wHeight;
extern float dt;

static int s_EntitiesCnfCapacity    = 32;
static int s_EntitiesNum            = 0;
static EntityDef* s_EntityDefs      = NULL;

static const char* triangle_texture_path   = "/res/static/textures/triangle.png";
static const char* square_texture_path     = "/res/static/textures/square.png";
static const char* circle_texture_path     = "/res/static/textures/circle.png";
static const char* castle_texture_path     = "/res/static/textures/castle.png";

// ! Allocates memory on heap !
static int alloc_entities_arr()
{
	s_EntitiesCnfCapacity *= 2;
	EntityDef* entity_defs_arr = realloc(s_EntityDefs, s_EntitiesCnfCapacity * sizeof *entity_defs_arr);

	if (NULL == entity_defs_arr)
	{
		PRINT_ERR("[entity]: Failed to allocate sufficient memory chunk for EntityDefs arr.");
		return TERMINATE_ERR_CODE;
	}

	s_EntityDefs = entity_defs_arr;

	return 0;
}

static int create_entity_def(EntityDef** dest, enum EntityType type)
{
    // TODO: How do we solve pointers invalidation problem? (use Registry)
	if (s_EntitiesNum >= s_EntitiesCnfCapacity)
	{
		int alloc_entities_arr_res = alloc_entities_arr();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != alloc_entities_arr_res, "[entity]: Failed to create entities arr.");
	}

	EntityDef* entity_def          = s_EntityDefs + s_EntitiesNum;
	entity_def->type               = type;
	entity_def->path               = NULL;
	entity_def->path_idx           = -1;
	entity_def->path_len           = 0;
	entity_def->state              = Entity_Setup;
	entity_def->drawable_handle    = -1;
	entity_def->collidable2D       = NULL;

	*dest = entity_def;

	s_EntitiesNum++;

	return 0;
}

static int add_triangle(EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	create_entity_def(dest, Entity_Triangle);

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, triangle_texture_path, TexType_RGBA, pos, scale, color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw triangle entity (empty quad drawable).");

	(*dest)->drawable_handle = drawable->handle;

	return 0;
}

static int add_square(EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	create_entity_def(dest, Entity_Square);

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, square_texture_path, TexType_RGBA, pos, scale, color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw square entity (empty quad drawable).");

	(*dest)->drawable_handle = drawable->handle;

	return 0;
}

static int add_circle(EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	create_entity_def(dest, Entity_Circle);

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, circle_texture_path, TexType_RGBA, pos, scale, color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw circle entity (empty quad drawable).");

	(*dest)->drawable_handle = drawable->handle;

	return 0;
}

static int add_castle(EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	create_entity_def(dest, Entity_Castle);

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, castle_texture_path, TexType_RGBA, pos, scale, color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw castle entity (empty quad drawable).");

	(*dest)->drawable_handle = drawable->handle;

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int add_entity(enum EntityType type, EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	if (NULL == s_EntityDefs)
	{
		int alloc_entities_arr_res = alloc_entities_arr();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != alloc_entities_arr_res, "[entity]: Failed to create entities arr.");
	}

	switch (type)
	{
	case Entity_Triangle:
		add_triangle(dest, pos, scale, color);
		break;
	case Entity_Square:
		add_square(dest, pos, scale, color);
		break;
	case Entity_Circle:
		add_circle(dest, pos, scale, color);
		break;
	case Entity_Castle:
		add_castle(dest, pos, scale, color);
		break;
	default:
		PRINT_ERR("[entity]: Unknown entity type.");
		break;
	}

	return 0;
}

// ! Allocates memory on heap !
int add_entity_path(EntityDef* dest, const PathDef* path, int path_len)
{
	PathSegment** path_ptr = malloc(path_len * sizeof *path_ptr);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != path_ptr, "[entity]: Failed to allocate sufficient memory chunk for PathSegment ptr.");

	dest->path = path_ptr;
	
	for (int i = 0; i < path_len; i++)
	{
		PathSegment* path_seg = malloc(sizeof *path_seg);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != path_seg, "[entity]: Failed to allocate sufficient memory chunk for PathSegment element.");

		dest->path[i]         = path_seg;
		dest->path[i]->start  = path->path_segment.start;
		dest->path[i]->end    = path->path_segment.end;

		path++;
	}

	dest->path_len = path_len;
	dest->path_idx = 0;

	return 0;
}

int move_entity(EntityDef* dest, float pos_x, float pos_y)
{
	DrawableDef* drawable = NULL;
	get_drawable_def(&drawable, dest->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity] Failed to fetch drawable for the entity.");

	drawable->transform.translation.x = pos_x;
	drawable->transform.translation.y = pos_y;

	if (NULL != dest->collidable2D && NULL != NULL != dest->collidable2D->collision_box)
	{
		move_collision_box2D(dest->collidable2D->collision_box, pos_x, pos_y);
	}

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

	return 0;
}

int resize_entity(EntityDef* dest, float scale_x, float scale_y)
{
	DrawableDef* drawable = NULL;
	get_drawable_def(&drawable, dest->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity] Failed to fetch drawable for the entity.");

	drawable->transform.scale.x = scale_x;
	drawable->transform.scale.y = scale_y;

	if (NULL != dest->collidable2D && NULL != dest->collidable2D->collision_box)
	{
		resize_collision_box2D(dest->collidable2D->collision_box, scale_x, scale_y);
	}

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

	return 0;
}

int find_entity_by_collidable2d(EntityDef** dest, const Collidable2D* collidable2D)
{
	EntityDef* entity = NULL;
	for (int i = 0; i < s_EntitiesNum; i++)
	{

	}

	return 0;
}

int entity_follow_path(EntityDef* entity)
{
	if (NULL == entity->path || 0 == entity->path_len)
	{
		return 0;
	}

	DrawableDef* drawable = NULL;
	get_drawable_def(&drawable, entity->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: DrawableDef was not found in Registry.");

	// TODO: If we later need to separate 'visible' (graphics) from 'active' (physics) - switch it here
	if (!drawable->visible)
		return 0;

	// Place at the start of the path
	if (entity->state == Entity_Setup && entity->path_len != 0)
	{
		entity->state = Entity_Moving;

		Vec3 starting_pos = { { entity->path[0]->start.x, entity->path[0]->start.y, drawable->transform.translation.z } };
		drawable->transform.translation = starting_pos;

		drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);
	}

	if (entity->state == Entity_Moving && entity->path_len != 0)
	{
		int path_idx = entity->path_idx;

		const Vec2* pos_start = &entity->path[path_idx]->start;
		const Vec2* pos_end = &entity->path[path_idx]->end;
	
		float pos_x_step = 0.f;
		float pos_y_step = 0.f;

		if (pos_end->x > pos_start->x)
		{
			pos_x_step = ENTITY_MOVEMENT_SPEED;
		}
		else if (pos_end->x < pos_start->x)
		{
			pos_x_step = -ENTITY_MOVEMENT_SPEED;
		}

		if (pos_end->y > pos_start->y)
		{
			pos_y_step = ENTITY_MOVEMENT_SPEED;
		}
		else if (pos_end->y < pos_start->y)
		{
			pos_y_step = -ENTITY_MOVEMENT_SPEED;
		}

		float new_pos_x = drawable->transform.translation.x + pos_x_step * dt;
		float new_pos_y = drawable->transform.translation.y + pos_y_step * dt;

		// TODO: Better way to do this?
		bool move_to_next_segment = false;
		if (pos_x_step >= 0 && pos_y_step >= 0)
		{
			move_to_next_segment = new_pos_x >= pos_end->x && new_pos_y >= pos_end->y;
		}
		else if (pos_x_step < 0 && pos_y_step >= 0)
		{
			move_to_next_segment = new_pos_x <= pos_end->x && new_pos_y >= pos_end->y;
		}
		else if (pos_x_step >= 0 && pos_y_step < 0)
		{
			move_to_next_segment = new_pos_x >= pos_end->x && new_pos_y <= pos_end->y;
		}
		else if (pos_x_step < 0 && pos_y_step < 0)
		{
			move_to_next_segment = new_pos_x <= pos_end->x && new_pos_y <= pos_end->y;
		}

		if (move_to_next_segment)
		{
			entity->path_idx++;

			drawable->transform.translation.x = pos_end->x;
			drawable->transform.translation.y = pos_end->y;

			if (entity->path_idx >= entity->path_len)
			{
				// Stop at the end of the path
				entity->state = Entity_Idle;
			}

			return 0;
		}

		drawable->transform.translation.x = new_pos_x;
		drawable->transform.translation.y = new_pos_y;

		drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

		if (NULL != entity->collidable2D && NULL != entity->collidable2D->collision_box)
		{
			move_collision_box2D(entity->collidable2D->collision_box, new_pos_x, new_pos_y);
		}
	}

	return 0;
}

void get_entities(EntityDef** dest)
{
	*dest = s_EntityDefs;
}

int get_entities_num()
{
	return s_EntitiesNum;
}

void entity_free_resources()
{
	for (int i = 0; i < s_EntitiesNum; i++)
	{
		EntityDef* entity_def = s_EntityDefs + i;

		for (int j = 0; j < entity_def->path_len; j++)
		{
			free(entity_def->path[j]);
		}

		if (NULL != entity_def->path)
		{
			free(entity_def->path);
		}
		if (NULL != entity_def->collidable2D)
		{
			if (NULL != entity_def->collidable2D->collision_box)
			{
				free(entity_def->collidable2D->collision_box);
			}

			free(entity_def->collidable2D);
		}
	}

	free(s_EntityDefs);
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
