#include "entity.h"
#include "physics.h"

static PhysicsEntitiesCollidedCb s_entitiesCollidedCb = NULL;

int is_collided_AABB(DrawableDef* first, DrawableDef* second)
{
    Vec3* translation_first   = &first->transform.translation;
    Vec3* scale_first         = &first->transform.scale;

    Vec3* translation_second  = &second->transform.translation;
    Vec3* scale_second        = &second->transform.scale;

    int x_collided = translation_first->x + scale_first->x >= translation_second->x - scale_second->x &&
        translation_second->x + scale_second->x >= translation_first->x - scale_first->x;
    int y_collided = translation_first->y + scale_first->y >= translation_second->y - scale_second->y &&
        translation_second->y + scale_second->y >= translation_first->y - scale_second->y;

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
        if (!first_entity->collidable)
            continue;

        for (int j = i + 1; j < entities_num; j++)
        {
            // TODO: Check visibility
            EntityDef* second_entity = entities + j;
            if (!second_entity->collidable)
                continue;
            if (first_entity->type == second_entity->type)
                continue;

            DrawableDef* first_drawable = NULL;
            get_drawable_def(&first_drawable, first_entity);

            DrawableDef* second_drawable = NULL;
            get_drawable_def(&second_drawable, second_entity);

            if (NULL == first_drawable || NULL == second_drawable)
            {
                // TODO: Report error
                continue;
            }

            // TODO: Replace with physics quontities? (e.g. pos, size?)
            int collides = is_collided_AABB(first_drawable, second_drawable);

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