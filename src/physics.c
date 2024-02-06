#include "physics.h"

int isCollidedAABB(EntityDef* first, EntityDef* second)
{
    int x_collided = first->physics->pos.x + first->physics->scale.x >= second->physics->pos.x &&
        second->physics->pos.x + second->physics->scale.x >= first->physics->pos.x;
    int y_collided = first->physics->pos.y + first->physics->scale.y >= second->physics->pos.y &&
        second->physics->pos.y + second->physics->scale.y >= first->physics->pos.y;

    return x_collided && y_collided;
}

int physics_step()
{
    EntityDef* entities = NULL;
    get_entities(&entities);

    int entities_num = get_entities_num();

    return 0;
}