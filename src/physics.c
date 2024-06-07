#include "physics.h"
#include "global_defs.h"
#include "utils.h"
#include "drawable_ops.h"

static PhysicsCollisionEventCbPtr s_CollisionBeginCbPtr = NULL;
static PhysicsCollisionEventCbPtr s_CollisionEndCbPtr = NULL;

static Collidable2D* s_Collidables2D = NULL;
static int s_2DCollidablesCapacity = 64; // TODO: Towers' collidables mem realloc problem here
static int s_2DCollidablesCount  = 0;

static int alloc_collidable2D_handles_arr()
{
    // TODO: Not very intuitive, rewrite
    s_2DCollidablesCapacity *= 2;
    Collidable2D* collidables_arr = realloc(s_Collidables2D, sizeof *s_Collidables2D * s_2DCollidablesCapacity);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidables_arr, "[physics]: Failed to allocate sufficient memory for collidables arr.");

    s_Collidables2D = collidables_arr;

    return 0;
}

int is_collided_AABB(const CollisionBox2D* first, const CollisionBox2D* second)
{
    if (NULL == first || NULL == second)
    {
        return 0;
    }

    Vec3* position_first        = &first->position;
    Vec3* size_first            = &first->size;

    Vec3* position_second       = &second->position;
    Vec3* size_second           = &second->size;

    int x_collided = position_first->x + size_first->x >= position_second->x - size_second->x &&
        position_second->x + size_second->x >= position_first->x - size_first->x;
    int y_collided = position_first->y + size_first->y >= position_second->y - size_second->y &&
        position_second->y + size_second->y >= position_first->y - size_first->y;

    return x_collided && y_collided;
}

void physics_bind_collision_begind_cb(PhysicsCollisionEventCbPtr cb)
{
    s_CollisionBeginCbPtr = cb;
}

void physics_bind_collision_end_cb(PhysicsCollisionEventCbPtr cb)
{
    s_CollisionEndCbPtr = cb;
}

int add_collidable2D(int* handle_dest, const Vec3* initial_pos, const Vec3* initial_size)
{
    if (NULL == s_Collidables2D || s_2DCollidablesCount >= s_2DCollidablesCapacity)
    {
        int alloc_handles_res = alloc_collidable2D_handles_arr();
        CHECK_EXPR_FAIL_RET_TERMINATE(alloc_handles_res != TERMINATE_ERR_CODE, "[physics]: Failed to create collidable 2D collidables arr.");
    }

    Collidable2D* collidable2D                       = s_Collidables2D + s_2DCollidablesCount;
    collidable2D->collision_box.collision_layer      = CollisionLayer_None;
    collidable2D->collision_box.collision_mask       = CollisionLayer_None;

    // TODO: Calcualte offset here, if a collision box is bigger than the drawable
    memcpy(&collidable2D->collision_box.position, initial_pos, sizeof(Vec3));
    memcpy(&collidable2D->collision_box.size, initial_size, sizeof(Vec3));

    // TODO: Should be configurated by client.
#if DRAW_COLLISION_BOX_BOUNDS
    collidable2D->collision_box.DEBUG_draw_bounds = 1;
    static const char* debug_quad_texture_path = "/res/static/textures/debug_quad.png";

    DrawableDef* debug_drawable = NULL;

    Vec4 debug_color = { { 1.f, 0.f, 0.f, 1.f } };
    draw_quad(&debug_drawable, debug_quad_texture_path, TexType_RGBA, &collidable2D->collision_box.position, &collidable2D->collision_box.size, &debug_color);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != debug_drawable, "[physics] Failed to create drawable for debug quad.");

    collidable2D->collision_box.DEBUG_bounds_drawable = debug_drawable;
#endif // DRAW_COLLISION_BOX_BOUNDS

    collidable2D->handle = s_2DCollidablesCount;
    collidable2D->collisions_detected = 0;

    memset(collidable2D->collision_handles, -1, MAX_COLLISION_HANDLES * sizeof(int));

    *handle_dest = s_2DCollidablesCount;

    s_2DCollidablesCount++;

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

int get_collidable2D(Collidable2D** dest, int handle)
{
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != s_Collidables2D, "[physics]: Collidable2D arr has not been initialized.");
    CHECK_EXPR_FAIL_RET_TERMINATE(handle >= 0 && handle < s_2DCollidablesCount, "[physics]: Collidable2D handle is out of bounds.");

    Collidable2D* collidable2D = s_Collidables2D + handle;
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[physics]: Collidable2D has not been found.");

    *dest = collidable2D;

    return 0;
}

int move_collision_box2D(CollisionBox2D* collision_box, float pos_x, float pos_y)
{
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collision_box, "[physics]: Collision box must be created before you move it.");

    // TODO: Is it OK to auto move collision box on drawable move?
    collision_box->position.x = pos_x;
    collision_box->position.y = pos_y;

#if DRAW_COLLISION_BOX_BOUNDS
    if (NULL != collision_box->DEBUG_bounds_drawable)
    {
        collision_box->DEBUG_bounds_drawable->transform.translation.x = pos_x;
        collision_box->DEBUG_bounds_drawable->transform.translation.y = pos_y;
        drawable_transform_ts(collision_box->DEBUG_bounds_drawable, COMMON_MODEL_UNIFORM_NAME);
    }
#endif // DRAW_COLLISION_BOX_BOUNDS

    return 0;
}

int resize_collision_box2D(CollisionBox2D* collision_box, float size_x, float size_y)
{
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collision_box, "[physics]: Collision box must be created before you resize it.");

    // TODO: Is it OK to auto resize collision box on drawable resize?
    collision_box->size.x = size_x;
    collision_box->size.y = size_y;

#if DRAW_COLLISION_BOX_BOUNDS
    if (NULL != collision_box->DEBUG_bounds_drawable)
    {
        collision_box->DEBUG_bounds_drawable->transform.scale.x = size_x;
        collision_box->DEBUG_bounds_drawable->transform.scale.y = size_y;
        drawable_transform_ts(collision_box->DEBUG_bounds_drawable, COMMON_MODEL_UNIFORM_NAME);
    }
#endif // DRAW_COLLISION_BOX_BOUNDS

    return 0;
}

// TODO: Add broad phase detection
// TODO: This needs to be reworked...
int physics_step()
{
    for (int i = 0; i < s_2DCollidablesCount; i++)
    {
        Collidable2D* first_collidable = s_Collidables2D + i;
        if (NULL == first_collidable || first_collidable->collision_box.collision_layer == CollisionLayer_None)
        {
            continue;
        }

        for (int j = 0; j < s_2DCollidablesCount; j++)
        {
            // Same entity - skip.
            if (i == j) 
            {
                continue;
            }
                
            Collidable2D* second_collidable = s_Collidables2D + j;
            if (NULL == second_collidable || second_collidable->collision_box.collision_layer == CollisionLayer_None)
            {
                continue;
            }

            if (
                (first_collidable->collision_box.collision_mask & second_collidable->collision_box.collision_layer) == 0 &&
                (second_collidable->collision_box.collision_mask & first_collidable->collision_box.collision_layer) == 0
            )
            {
                continue;
            }

            int collides = is_collided_AABB(&first_collidable->collision_box, &second_collidable->collision_box);
            if (collides)
            {
                if (first_collidable->collisions_detected < MAX_COLLISION_HANDLES && second_collidable->collisions_detected < MAX_COLLISION_HANDLES)
                {
                    bool already_collides = false;
                    int next_free_spot_first = -1;
                    for (int ii = 0; ii < MAX_COLLISION_HANDLES; ii++)
                    {
                        int collidable_handle = first_collidable->collision_handles[ii];
                        if (next_free_spot_first == -1 && collidable_handle == -1)
                        {
                            next_free_spot_first = ii;
                        }
                        if (second_collidable->handle == collidable_handle)
                        {
                            already_collides = true;
                            break;
                        }
                    }
                    int next_free_spot_second = -1;
                    for (int ii = 0; ii < MAX_COLLISION_HANDLES; ii++)
                    {
                        int collidable_handle = second_collidable->collision_handles[ii];
                        if (next_free_spot_second == -1 && collidable_handle == -1)
                        {
                            next_free_spot_second = ii;
                            break;
                        }
                    }

                    if (!already_collides)
                    {
                        first_collidable->collision_handles[next_free_spot_first] = second_collidable->handle;
                        first_collidable->collisions_detected += 1;

                        second_collidable->collision_handles[next_free_spot_second] = first_collidable->handle;
                        second_collidable->collisions_detected += 1;
                        
                        if (NULL != s_CollisionBeginCbPtr)
                        {
                            (*s_CollisionBeginCbPtr)(first_collidable, second_collidable);
                        }
                    }
                }
            }
            else
            {
                bool trigger_uncollided_first = false;
                for (int jj = 0; jj < MAX_COLLISION_HANDLES; jj++)
                {
                    int collidable_handle = first_collidable->collision_handles[jj];
                    if (second_collidable->handle == collidable_handle)
                    {
                        trigger_uncollided_first = true;

                        first_collidable->collision_handles[jj] = -1;
                        first_collidable->collisions_detected -= 1;

                        break;
                    }
                }

                bool trigger_uncollided_second = false;
                for (int kk = 0; kk < MAX_COLLISION_HANDLES; kk++)
                {
                    int collidable_handle = second_collidable->collision_handles[kk];
                    if (first_collidable->handle == collidable_handle)
                    {
                        trigger_uncollided_second = true;

                        second_collidable->collision_handles[kk] = -1;
                        second_collidable->collisions_detected -= 1;

                        break;
                    }
                }

                if (trigger_uncollided_first && trigger_uncollided_second)
                {
                    if (NULL != s_CollisionEndCbPtr)
                    {
                        (*s_CollisionEndCbPtr)(first_collidable, second_collidable);
                    }
                }
            }
        }
    }

    return 0;
}

void physics_free_resources()
{
    free(s_Collidables2D);
}
