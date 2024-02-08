#include "physics.h"

// TODO: Use physics 'width', 'height' and 'pos' instead of graphics' pos and scale
int isCollidedAABB(EntityDef* first, EntityDef* second)
{
    int x_collided = first->physics->pos.x + first->physics->scale.x >= second->physics->pos.x - second->physics->scale.x &&
        second->physics->pos.x + second->physics->scale.x >= first->physics->pos.x - first->physics->scale.x;
    int y_collided = first->physics->pos.y + first->physics->scale.y >= second->physics->pos.y - second->physics->scale.y &&
        second->physics->pos.y + second->physics->scale.y >= first->physics->pos.y - second->physics->scale.y;

    return x_collided && y_collided;
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
        if (!first_entity->physics->collidable)
            continue;

        for (int j = i + 1; j < entities_num; j++)
        {
            // TODO: Check visibility
            EntityDef* second_entity = entities + j;
            if (!second_entity->physics->collidable)
                continue;
            if (first_entity->type == second_entity->type)
                continue;

            int collides = isCollidedAABB(first_entity, second_entity);

            if (collides)
            {
                DrawableDef* first_drawable = NULL;
                get_drawable_def(&first_drawable, first_entity);
                if (NULL != first_drawable && first_entity->type == Entity_Castle)
                {
                    Vec4 color_vec = { { 1.f, 0.f, 0.f, 1.f } };
                    add_uniform_vec4f(first_drawable->shader_prog, "UColor", &color_vec);
                }

                DrawableDef* second_drawable = NULL;
                get_drawable_def(&second_drawable, second_entity);
                if (NULL != second_drawable && second_entity->type == Entity_Castle)
                {
                    Vec4 color_vec = { { 1.f, 0.f, 0.f, 1.f } };
                    add_uniform_vec4f(first_drawable->shader_prog, "UColor", &color_vec);
                }
            }
            
        }
    }

    return 0;
}