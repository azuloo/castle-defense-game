#include "entity.h"
#include "physics.h"
#include "global_defs.h"
#include "utils.h"
#include "drawable_ops.h"

static PhysicsEntitiesCollidedCb s_entitiesCollidedCb = NULL;

int is_collided_AABB(EntityDef* first, EntityDef* second)
{
    if (NULL == first->collision_box || NULL == second->collision_box)
    {
        return 0;
    }

    Vec3* position_first        = &first->collision_box->position;
    Vec3* size_first            = &first->collision_box->size;

    Vec3* position_second       = &second->collision_box->position;
    Vec3* size_second           = &second->collision_box->size;

    int x_collided = position_first->x + size_first->x >= position_second->x - size_second->x &&
        position_second->x + size_second->x >= position_first->x - size_first->x;
    int y_collided = position_first->y + size_first->y >= position_second->y - size_second->y &&
        position_second->y + size_second->y >= position_first->y - size_first->y;

    return x_collided && y_collided;
}

void physics_bind_entities_collided_cb(PhysicsEntitiesCollidedCb cb)
{
    s_entitiesCollidedCb = cb;
}

int add_collision_box2D(CollisionBox2D** dest, const Vec3* initial_pos, const Vec3* initial_size)
{
    CollisionBox2D* collision_box = malloc(sizeof *collision_box);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collision_box, "[physics]: Failed to allocate sufficient memory chunk for CollisionBox2D.");

    collision_box->collision_layer = CollistionLayer_None;

    // TODO: Calcualte offset here, if a collision box is bigger than the drawable
    memcpy(&collision_box->position, initial_pos, sizeof(Vec3));
    memcpy(&collision_box->size, initial_size, sizeof(Vec3));

    // TODO: Should be configurated by client.
#if DEBUG
    collision_box->DEBUG_draw_bounds = 1;
    static const char* debug_quad_texture_path = "/res/static/textures/debug_quad.png";

    DrawableDef* debug_drawable = NULL;

    Vec4 debug_color = { { 1.f, 0.f, 0.f, 1.f } };
    draw_quad(&debug_drawable, debug_quad_texture_path, TexType_RGBA, &collision_box->position, &collision_box->size, &debug_color);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != debug_drawable, "[entity] Failed to create drawable for debug quad.");

   collision_box->DEBUG_bounds_drawable = debug_drawable;
#endif // DEBUG

   *dest = collision_box;

    return 0;
}

void add_collision_layer2D(CollisionBox2D* collision_box, uint16_t layer)
{
    CHECK_EXPR_FAIL_RET(NULL != collision_box, "[physics]: Collision box must be created before you assign a layer to it.");

    collision_box->collision_layer = layer;
}

void add_collision_mask2D(CollisionBox2D* collision_box, uint16_t mask)
{
    CHECK_EXPR_FAIL_RET(NULL != collision_box, "[physics]: Collision box must be created before you assign a mask to it.");

    collision_box->collision_mask |= mask;
}

int move_collision_box2D(CollisionBox2D* collision_box, float pos_x, float pos_y)
{
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collision_box, "[physics]: Collision box must be created before you move it.");

    // TODO: Is it OK to auto move collision box on drawable move?
    collision_box->position.x = pos_x;
    collision_box->position.y = pos_y;

    if (NULL != collision_box->DEBUG_bounds_drawable)
    {
        collision_box->DEBUG_bounds_drawable->transform.translation.x = pos_x;
        collision_box->DEBUG_bounds_drawable->transform.translation.y = pos_y;
        drawable_transform_ts(collision_box->DEBUG_bounds_drawable, COMMON_MODEL_UNIFORM_NAME);
    }

    return 0;
}

int resize_collision_box2D(CollisionBox2D* collision_box, float size_x, float size_y)
{
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collision_box, "[physics]: Collision box must be created before you resize it.");

    // TODO: Is it OK to auto resize collision box on drawable resize?
    collision_box->size.x = size_x;
    collision_box->size.y = size_y;

    if (NULL != collision_box->DEBUG_bounds_drawable)
    {
        collision_box->DEBUG_bounds_drawable->transform.scale.x = size_x;
        collision_box->DEBUG_bounds_drawable->transform.scale.y = size_y;
        drawable_transform_ts(collision_box->DEBUG_bounds_drawable, COMMON_MODEL_UNIFORM_NAME);
    }

    return 0;
}

// TODO: Add broad phase detection
int physics_step()
{
    EntityDef* entities = NULL;
    get_entities(&entities);

    int entities_num = get_entities_num();
    for (int i = 0; i < entities_num; i++)
    {
        // TODO: Check visibility
        EntityDef* first_entity = entities + i;
        if (NULL == first_entity->collision_box || first_entity->collision_box->collision_layer == CollistionLayer_None)
            continue;

        for (int j = 0; j < entities_num; j++)
        {
            if (i == j) // Same entity - skip.
                continue;
            // TODO: Check visibility
            EntityDef* second_entity = entities + j;
            if (NULL == second_entity->collision_box || second_entity->collision_box->collision_layer == CollistionLayer_None)
                continue;

            if (
                (first_entity->collision_box->collision_mask & second_entity->collision_box->collision_layer) == 0 &&
                (second_entity->collision_box->collision_mask & first_entity->collision_box->collision_layer) == 0
            )
            {
                continue;
            }

            int collides = is_collided_AABB(first_entity, second_entity);
            if (collides)
            {
                if (NULL != s_entitiesCollidedCb)
                {
                    (*s_entitiesCollidedCb)(first_entity, second_entity);
                }
            }
        }
    }

    return 0;
}