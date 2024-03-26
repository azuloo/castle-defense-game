#include "entity.h"
#include "physics.h"

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