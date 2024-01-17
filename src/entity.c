#include "entity.h"
#include "file_reader.h"
#include "utils.h"
#include "obj_registry.h"

#include <stdlib.h>
#include <stdbool.h>

extern float wWidth;
extern float wHeight;
extern float dt;

static int add_entity_common(EntityDef* dest, const DrawBufferData* draw_buf_data, const char* texture_path, const Vec3* new_pos, const Vec3* new_scale)
{
	static const char* vertex_shader_path = "/res/static/shaders/entity_vert.txt";
	static const char* fragment_shader_path = "/res/static/shaders/entity_frag.txt";

	EntryCnf* entry = create_entry();
	if (NULL == entry)
	{
		PRINT_ERR("[static_env]: Failed to create entry.");
		return TERMINATE_ERR_CODE;
	}

	dest->entry_handle = entry->handle;

	char texture_buf[256];
	get_file_path(texture_path, texture_buf, 256);

	int create_texture_2D_res = create_texture_2D(texture_buf, &entry->texture, TexType_RGBA);
	if (TERMINATE_ERR_CODE == create_texture_2D_res)
	{
		PRINT_ERR("[static_env]: Failed to add env texute.");
		return TERMINATE_ERR_CODE;
	}

	int add_res = add_element(entry, draw_buf_data, vertex_shader_path, fragment_shader_path);
	if (TERMINATE_ERR_CODE == add_res)
	{
		PRINT_ERR("[entity]: Failed to add env element.");
		return TERMINATE_ERR_CODE;
	}

	add_entry_attribute(entry, 3);
	add_entry_attribute(entry, 2);

	apply_entry_attributes(entry);

	TransformDef* transform = malloc(sizeof *transform);
	if (NULL == transform)
	{
		PRINT_ERR("[static_env]: Failed to allocate sufficient memory chunk for TransformDef.");
		return TERMINATE_ERR_CODE;
	}

	transform->pos = *new_pos;
	transform->scale = *new_scale;
	memset(&transform->rotation, 0, sizeof &transform->rotation);

	dest->transform = transform;

	entry->matrices->model = IdentityMat;
	scale(&entry->matrices->model, new_scale->x, new_scale->y, new_scale->z);
	translate(&entry->matrices->model, new_pos->x, new_pos->y, new_pos->z);
	add_uniform_mat4f(entry->shader_prog, "model", &entry->matrices->model);
	
	entry->matrices->projection = ortho(0.f, wWidth, 0.f, wHeight, -1.f, 1.f);
	add_uniform_mat4f(entry->shader_prog, "projection", &entry->matrices->projection);

	return 0;
}

static int add_triangle(EntityDef** dest)
{
	static const float vertices[] = {
		// Position           // Texture
		0.f, 1.f, 0.f,        0.5f, 1.f,
		-1.f, -1.f, 0.f,      0.f, 0.f,
		1.f, -1.f, 0.0f,      1.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 2
	};

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices) / sizeof(vertices[0]);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices) / sizeof(indices[0]);

	EntityDef* entity_def = malloc(sizeof *entity_def);
	if (NULL == entity_def)
	{
		PRINT_ERR("[entity]: Failed to allocate sufficient memory chunk for EntityDef.");
		return TERMINATE_ERR_CODE;
	}

	*dest = entity_def;
	entity_def->type          = Entity_Triangle;
	entity_def->transform     = NULL;
	entity_def->path          = NULL;
	entity_def->path_idx      = -1;
	entity_def->path_len      = 0;
	entity_def->state         = Entity_Setup;
	entity_def->entry_handle  = -1;

	Vec3 tri_pos = { { 600.f, (float)wHeight / 2.f, 0.2f } };
	Vec3 tri_scale = { { 35.f, 35.f, 1.f } };

	const char* texture_path = "/res/static/textures/triangle.png";
	add_entity_common(*dest, &draw_buf_data, texture_path, &tri_pos, &tri_scale);

	return 0;
}

static int add_square(EntityDef** dest)
{
	static const float vertices[] = {
		// Position           // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices) / sizeof(vertices[0]);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices) / sizeof(indices[0]);

	EntityDef* entity_def = malloc(sizeof *entity_def);
	if (NULL == entity_def)
	{
		PRINT_ERR("[entity]: Failed to allocate sufficient memory chunk for EntityDef.");
		return TERMINATE_ERR_CODE;
	}

	*dest = entity_def;
	entity_def->type          = Entity_Square;
	entity_def->transform     = NULL;
	entity_def->path          = NULL;
	entity_def->path_idx      = -1;
	entity_def->path_len      = 0;
	entity_def->state         = Entity_Setup;
	entity_def->entry_handle  = -1;

	Vec3 sq_pos = { { 400.f, (float)wHeight / 2.f, 0.2f } };
	Vec3 sq_scale = { { 35.f, 35.f, 1.f } };

	const char* texture_path = "/res/static/textures/square.png";
	add_entity_common(*dest, &draw_buf_data, texture_path, &sq_pos, &sq_scale);

	return 0;
}

static int add_circle(EntityDef** dest)
{
	static const float vertices[] = {
		// Position           // Texture
		-1.f, 1.f, 0.f,       0.f, 1.f,
		1.f, 1.f, 0.f,        1.f, 1.f,
		1.f, -1.f, 0.0f,      1.f, 0.f,
		-1.f, -1.f, 0.0f,     0.f, 0.f
	};

	static unsigned int indices[] = {
		0, 1, 3,
		2, 3, 1
	};

	DrawBufferData draw_buf_data;
	draw_buf_data.vertices = vertices;
	draw_buf_data.vertices_len = sizeof(vertices) / sizeof(vertices[0]);
	draw_buf_data.indices = indices;
	draw_buf_data.indices_len = sizeof(indices) / sizeof(indices[0]);

	EntityDef* entity_def = malloc(sizeof *entity_def);
	if (NULL == entity_def)
	{
		PRINT_ERR("[entity]: Failed to allocate sufficient memory chunk for EntityDef.");
		return TERMINATE_ERR_CODE;
	}

	*dest = entity_def;
	entity_def->type          = Entity_Circle;
	entity_def->transform     = NULL;
	entity_def->path          = NULL;
	entity_def->path_idx      = -1;
	entity_def->path_len      = 0;
	entity_def->state         = Entity_Setup;
	entity_def->entry_handle  = -1;

	Vec3 sq_pos = { { 500.f, (float)wHeight / 2.f, 0.2f } };
	Vec3 sq_scale = { { 35.f, 35.f, 1.f } };

	const char* texture_path = "/res/static/textures/circle.png";
	const EntryCnf* entry = add_entity_common(*dest , &draw_buf_data, texture_path, &sq_pos, &sq_scale);

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int add_entity(enum EntityType type, EntityDef** dest)
{
	switch (type)
	{
	case Entity_Triangle:
		add_triangle(dest);
		break;
	case Entity_Square:
		add_square(dest);
		break;
	case Entity_Circle:
		add_circle(dest);
		break;
	default:
		PRINT_ERR("[entity]: Unknown entity type.");
		break;
	}

	return 0;
}

int add_entity_path(EntityDef* dest, const PathSegment* path, int path_len)
{
	PathSegment* path_ptr = malloc(path_len * sizeof *path_ptr);
	if (NULL == path_ptr)
	{
		PRINT_ERR("[entity]: Failed to allocate sufficient memory chunk for PathSegment ptr.");
		return TERMINATE_ERR_CODE;
	}

	dest->path = path_ptr;
	
	for (int i = 0; i < path_len; i++)
	{
		PathSegment* path_seg = malloc(sizeof *path_seg);
		if (NULL == path_seg)
		{
			PRINT_ERR("[entity]: Failed to allocate sufficient memory chunk for PathSegment element.");
			return TERMINATE_ERR_CODE;
		}

		dest->path[i]         = path_seg;
		dest->path[i]->start  = path[i].start;
		dest->path[i]->end    = path[i].end;
	}

	dest->path_len = path_len;
	dest->path_idx = 0;

	return 0;
}

int entity_follow_path(EntityDef* entity)
{
	EntryCnf* entry = (EntryCnf*) GET_FROM_REGISTRY(&entity->entry_handle);
	if (NULL == entry)
	{
		PRINT_ERR("[entity]: Failed to fetch EntryCnf from registry.");
		return TERMINATE_ERR_CODE;
	}

	// Place at the start of the path
	if (entity->state == Entity_Setup && entity->path_len != 0)
	{
		Vec3 starting_pos = { { entity->path[0]->start.x, entity->path[0]->start.y, entity->transform->pos.z } };

		entry->matrices->model = IdentityMat;
		scale(&entry->matrices->model, entity->transform->scale.x, entity->transform->scale.y, entity->transform->scale.z);
		translate(&entry->matrices->model, starting_pos.x, starting_pos.y, entity->transform->pos.z);
		add_uniform_mat4f(entry->shader_prog, "model", &entry->matrices->model);

		entity->transform->pos = starting_pos;
		entity->state = Entity_Moving;
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

		float new_pos_x = entity->transform->pos.x + pos_x_step * dt;
		float new_pos_y = entity->transform->pos.y + pos_y_step * dt;

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

			entity->transform->pos.x = pos_end->x;
			entity->transform->pos.y = pos_end->y;

			if (entity->path_idx >= entity->path_len)
			{
				// Repeat the path
				entity->path_idx = 0;
				// Stop at the end of the path
				//entity->state = Entity_Idle;
			}

			return;
		}

		// TODO: Move to separate func
		entry->matrices->model = IdentityMat;
		scale(&entry->matrices->model, entity->transform->scale.x, entity->transform->scale.y, entity->transform->scale.z);
		translate(&entry->matrices->model, new_pos_x, new_pos_y, entity->transform->pos.z);
		add_uniform_mat4f(entry->shader_prog, "model", &entry->matrices->model);

		entity->transform->pos.x = new_pos_x;
		entity->transform->pos.y = new_pos_y;
	}
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
