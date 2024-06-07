#include "entity.h"
#include "physics.h"
#include "file_reader.h"
#include "utils.h"
#include "global_defs.h"
#include "drawable_ops.h"
#include "file_reader.h"
#include "map/map_mgr.h"

#include <stdlib.h>
#include <stdbool.h>

extern int wHeight;
extern float dt;

extern int xWOffset;
extern int yWOffset;

static int s_EntitiesCnfCapacity    = 32;
static int s_EntitiesNum            = 0;
static EntityDef* s_EntityDefs      = NULL;

static const char* triangle_texture_path   = "/res/static/textures/triangle.png";
static const char* square_texture_path     = "/res/static/textures/square.png";
static const char* circle_texture_path     = "/res/static/textures/circle.png";

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

	EntityDef* entity_def             = s_EntityDefs + s_EntitiesNum;
	entity_def->type                  = type;
	entity_def->path                  = NULL;
	entity_def->path_idx              = -1;
	entity_def->path_len              = 0;
	entity_def->segment_percent       = 0.f;
	entity_def->state                 = EntityState_Setup;
	entity_def->initial_speed         = ENTITY_MOVEMENT_SPEED;
	entity_def->speed                 = ENTITY_MOVEMENT_SPEED;
	entity_def->drawable_handle       = -1;
	entity_def->collidable2D_handle   = -1;

	memset(&entity_def->direction, 0, sizeof(Vec2));

	*dest = entity_def;

	s_EntitiesNum++;

	return 0;
}

static int add_triangle(EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	create_entity_def(dest, EntityType_Triangle);

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, triangle_texture_path, TexType_RGBA, pos, scale, color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw triangle entity (empty quad drawable).");

	(*dest)->drawable_handle = drawable->handle;

	return 0;
}

static int add_square(EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	create_entity_def(dest, EntityType_Square);

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, square_texture_path, TexType_RGBA, pos, scale, color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw square entity (empty quad drawable).");

	(*dest)->drawable_handle = drawable->handle;

	return 0;
}

static int add_circle(EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	create_entity_def(dest, EntityType_Circle);

	DrawableDef* drawable = NULL;
	draw_quad(&drawable, circle_texture_path, TexType_RGBA, pos, scale, color);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity]: Failed to draw circle entity (empty quad drawable).");

	(*dest)->drawable_handle = drawable->handle;

	return 0;
}

int add_entity(enum EntityType type, EntityDef** dest, const Vec3* pos, const Vec3* scale, const Vec4* color)
{
	if (NULL == s_EntityDefs)
	{
		int alloc_entities_arr_res = alloc_entities_arr();
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != alloc_entities_arr_res, "[entity]: Failed to create entities arr.");
	}

	switch (type)
	{
	case EntityType_Triangle:
		add_triangle(dest, pos, scale, color);
		break;
	case EntityType_Square:
		add_square(dest, pos, scale, color);
		break;
	case EntityType_Circle:
		add_circle(dest, pos, scale, color);
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
	PathSegment* path_ptr = NULL;
	if (NULL == dest->path)
	{
		path_ptr = malloc(path_len * sizeof * path_ptr);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != path_ptr, "[entity]: Failed to allocate sufficient memory chunk for PathSegment ptr.");
		dest->path = path_ptr;
	}

	for (int i = 0; i < path_len; i++)
	{
		PathSegment* path_seg_dest = dest->path + i;
		PathDef* path_def_src = path + i;

		memcpy(path_seg_dest, &path_def_src->path_segment, sizeof(PathSegment));
	}

	dest->path_len = path_len;

	// If the path was re-assigned (e.g. after scaling) we don't need to reset the path_idx.
	if (dest->path_idx == -1)
	{
		dest->path_idx = 0;
	}

	return 0;
}

int move_entity(EntityDef* dest, float pos_x, float pos_y)
{
	DrawableDef* drawable = NULL;
	get_drawable_def(&drawable, dest->drawable_handle);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[entity] Failed to fetch drawable for the entity.");

	drawable->transform.translation.x = pos_x;
	drawable->transform.translation.y = pos_y;

	if (-1 != dest->collidable2D_handle)
	{
		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, dest->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[entity] Failed to fetch Collidable2D for the entity.");

		move_collision_box2D(&collidable2D->collision_box, pos_x, pos_y);
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

	if (-1 != dest->collidable2D_handle)
	{
		Collidable2D* collidable2D = NULL;
		get_collidable2D(&collidable2D, dest->collidable2D_handle);
		CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[entity] Failed to fetch Collidable2D for the entity.");

		resize_collision_box2D(&collidable2D->collision_box, scale_x, scale_y);
	}

	drawable_transform_ts(drawable, COMMON_MODEL_UNIFORM_NAME);

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
	{
		return 0;
	}

	if (entity->state == EntityState_Setup && entity->path_len != 0)
	{
		entity->state = EntityState_Moving;

		Vec3 starting_pos = { { entity->path->start.x, entity->path->start.y, drawable->transform.translation.z } };
		drawable->transform.translation = starting_pos;

		move_entity(entity, starting_pos.x, starting_pos.y);
	}

	if (entity->state == EntityState_OnWindowResize && entity->path_len != 0)
	{
		int path_idx = entity->path_idx;
		const PathSegment* path_seg = entity->path + path_idx;

		float path_pos_x = 0.f;
		float path_pos_y = 0.f;

		if (vec2_equals(entity->direction, Vec2_RIGHT))
		{
			path_pos_x = math_lerp(path_seg->start.x, path_seg->end.x, entity->segment_percent);
			path_pos_y = math_lerp(path_seg->start.y, path_seg->end.y, entity->segment_percent);
		}
		else if (vec2_equals(entity->direction, Vec2_LEFT))
		{
			path_pos_x = math_lerp(path_seg->end.x, path_seg->start.x, entity->segment_percent);
			path_pos_y = math_lerp(path_seg->end.y, path_seg->start.y, entity->segment_percent);
		}
		else if (vec2_equals(entity->direction, Vec2_UP))
		{
			path_pos_x = math_lerp(path_seg->start.x, path_seg->end.x, entity->segment_percent);
			path_pos_y = math_lerp(path_seg->start.y, path_seg->end.y, entity->segment_percent);
		}
		else if (vec2_equals(entity->direction, Vec2_DOWN))
		{
			path_pos_x = math_lerp(path_seg->end.x, path_seg->start.x, entity->segment_percent);
			path_pos_y = math_lerp(path_seg->end.y, path_seg->start.y, entity->segment_percent);
		}

		Vec3 starting_pos = { { path_pos_x, path_pos_y, drawable->transform.translation.z } };
		drawable->transform.translation = starting_pos;

		move_entity(entity, starting_pos.x, starting_pos.y);

		entity->state = EntityState_Moving;
	}

	if (entity->state == EntityState_Moving && entity->path_len != 0)
	{
		int path_idx = entity->path_idx;
		const PathSegment* path_seg = entity->path + path_idx;

		const Vec2* pos_start = &path_seg->start;
		const Vec2* pos_end = &path_seg->end;
	
		float pos_x_step = 0.f;
		float pos_y_step = 0.f;

		if (pos_end->x > pos_start->x)
		{
			entity->direction = Vec2_RIGHT;
		}
		else if (pos_end->x < pos_start->x)
		{
			entity->direction = Vec2_LEFT;
		}
		if (pos_end->y > pos_start->y)
		{
			entity->direction = Vec2_UP;
		}
		else if (pos_end->y < pos_start->y)
		{
			entity->direction = Vec2_DOWN;
		}

		float delta_pos_x = entity->speed * entity->direction.x * dt;
		float delta_pos_y = entity->speed * entity->direction.y * dt;

		float new_pos_x = drawable->transform.translation.x + delta_pos_x;
		float new_pos_y = drawable->transform.translation.y + delta_pos_y;

		bool move_to_next_segment = false;
		float segment_percent = 0.f;
		float total_distance = 0.f;
		float entity_distance = 0.f;

		if (vec2_equals(entity->direction, Vec2_RIGHT))
		{
			if (new_pos_x > pos_end->x)
			{
				move_to_next_segment = true;
			}

			total_distance = fabs(path_seg->end.x - path_seg->start.x);
			entity_distance = fabs(new_pos_x - path_seg->start.x);
			segment_percent = entity_distance / total_distance;
		}
		else if (vec2_equals(entity->direction, Vec2_LEFT))
		{
			if (new_pos_x < pos_end->x)
			{
				move_to_next_segment = true;
			}

			total_distance = (float)fabs(path_seg->end.x - path_seg->start.x);
			entity_distance = (float)fabs(new_pos_x - path_seg->start.x);
			segment_percent = entity_distance / total_distance;
		}
		else if (vec2_equals(entity->direction, Vec2_UP))
		{
			if (new_pos_y > pos_end->y)
			{
				move_to_next_segment = true;
			}

			total_distance = fabs(path_seg->end.y - path_seg->start.y);
			entity_distance = fabs(new_pos_y - path_seg->start.y);
			segment_percent = entity_distance / total_distance;
		}
		else if (vec2_equals(entity->direction, Vec2_DOWN))
		{
			if (new_pos_y < pos_end->y)
			{
				move_to_next_segment = true;
			}
		
			total_distance = fabs(path_seg->end.y - path_seg->start.y);
			entity_distance = fabs(new_pos_y - path_seg->start.y);
			segment_percent = entity_distance / total_distance;
		}
		
		entity->segment_percent = segment_percent;

		if (move_to_next_segment)
		{
			entity->path_idx++;
			entity->segment_percent = 0.f;

			drawable->transform.translation.x = pos_end->x;
			drawable->transform.translation.y = pos_end->y;

			if (entity->path_idx >= entity->path_len)
			{
				// Stop at the end of the path.
				entity->state = EntityState_Idle;
			}

			return 0;
		}

		move_entity(entity, new_pos_x, new_pos_y);
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

		if (NULL != entity_def->path)
		{
			free(entity_def->path);
		}
	}

	free(s_EntityDefs);
}
