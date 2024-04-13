#include "physics.h"
#include "global_defs.h"
#include "utils.h"
#include "drawable_ops.h"
#include "obj_registry.h"

static PhysicsCollisionEventCbPtr s_CollisionEventCbPtr = NULL;

static int* s_Collidable2DHandels    = NULL;
static int s_2DHandlesCapacity       = 16;
static int s_2DHandlesCount          = 0;

static int alloc_collidable2D_handles_arr()
{
    // TODO: Not very intuitive, rewrite
    s_2DHandlesCapacity *= 2;
    int* handles_arr = realloc(s_Collidable2DHandels, sizeof *s_Collidable2DHandels * s_2DHandlesCapacity);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != handles_arr, "[physics]: Failed to allocate sufficient memory for collidable handles.");

    s_Collidable2DHandels = handles_arr;

    return 0;
}

static int register_collidable2D(Collidable2D* collidable)
{
    if (NULL == s_Collidable2DHandels || s_2DHandlesCount >= s_2DHandlesCapacity)
    {
        int alloc_handles_res = alloc_collidable2D_handles_arr();
        CHECK_EXPR_FAIL_RET_TERMINATE(alloc_handles_res != TERMINATE_ERR_CODE, "[physics]: Failed to create collidable 2D handles arr.");
    }

    int registry_handle = -1;
    REGISTER_OBJ(collidable, &registry_handle)
    CHECK_EXPR_FAIL_RET_TERMINATE(registry_handle != -1, "[physics]: Faild to register collidable.");

    int* collidable_handle = s_Collidable2DHandels + s_2DHandlesCount;
    *collidable_handle = registry_handle;

    collidable->handle = *collidable_handle;

    s_2DHandlesCount++;

    return 0;
}

static int get_collidable2D(Collidable2D** dest, int idx)
{
    CHECK_EXPR_FAIL_RET_TERMINATE(idx >= 0 && idx < s_2DHandlesCount, "[physics]: Collidable2D idx is out of bounds.");
    int* collidable_handle = s_Collidable2DHandels + idx;
    CHECK_EXPR_FAIL_RET_TERMINATE(collidable_handle != NULL, "[physics]: Faild to fetch collidable handle.");

    Collidable2D* collidable = (Collidable2D*) GET_FROM_REGISTRY(*collidable_handle);
    CHECK_EXPR_FAIL_RET_TERMINATE(collidable != NULL, "[physics]: Faild to fetch collidable from the registry.");

    *dest = collidable;

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

void physics_bind_collision_event_cb(PhysicsCollisionEventCbPtr cb)
{
    s_CollisionEventCbPtr = cb;
}

int add_collidable2D(Collidable2D** dest, const Vec3* initial_pos, const Vec3* initial_size)
{
    Collidable2D* collidable2D = malloc(sizeof *collidable2D);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collidable2D, "[physics]: Failed to allocate sufficient memory chunk for Collidable2D.");

    CollisionBox2D* collision_box2D = malloc(sizeof *collision_box2D);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != collision_box2D, "[physics]: Failed to allocate sufficient memory chunk for CollisionBox2D.");

    collision_box2D->collision_layer   = CollisionLayer_None;
    collision_box2D->collision_mask    = CollisionLayer_None;

    // TODO: Calcualte offset here, if a collision box is bigger than the drawable
    memcpy(&collision_box2D->position, initial_pos, sizeof(Vec3));
    memcpy(&collision_box2D->size, initial_size, sizeof(Vec3));

    // TODO: Should be configurated by client.
#if DEBUG
    collision_box2D->DEBUG_draw_bounds = 1;
    static const char* debug_quad_texture_path = "/res/static/textures/debug_quad.png";

    DrawableDef* debug_drawable = NULL;

    Vec4 debug_color = { { 1.f, 0.f, 0.f, 1.f } };
    draw_quad(&debug_drawable, debug_quad_texture_path, TexType_RGBA, &collision_box2D->position, &collision_box2D->size, &debug_color);
    CHECK_EXPR_FAIL_RET_TERMINATE(NULL != debug_drawable, "[entity] Failed to create drawable for debug quad.");

    collision_box2D->DEBUG_bounds_drawable = debug_drawable;
#endif // DEBUG

    collidable2D->handle            = -1;
    collidable2D->collision_box     = collision_box2D;
    collidable2D->collision_state   = CollisionState_Uncollided;

   *dest = collidable2D;

   register_collidable2D(collidable2D);

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
    for (int i = 0; i < s_2DHandlesCount; i++)
    {
        Collidable2D* first_collidable = NULL;
        get_collidable2D(&first_collidable, i);
        if (NULL == first_collidable || NULL == first_collidable->collision_box || first_collidable->collision_box->collision_layer == CollisionLayer_None)
        {
            continue;
        }

        for (int j = 0; j < s_2DHandlesCount; j++)
        {
            // Same entity - skip.
            if (i == j) 
            {
                continue;
            }
                
            Collidable2D* second_collidable = NULL;
            get_collidable2D(&second_collidable, j);
            if (NULL == second_collidable || NULL == second_collidable->collision_box || second_collidable->collision_box->collision_layer == CollisionLayer_None)
            {
                continue;
            }

            if (
                (first_collidable->collision_box->collision_mask & second_collidable->collision_box->collision_layer) == 0 &&
                (second_collidable->collision_box->collision_mask & first_collidable->collision_box->collision_layer) == 0
            )
            {
                continue;
            }

            int collides = is_collided_AABB(first_collidable->collision_box, second_collidable->collision_box);
            if (collides && ((first_collidable->collision_state & CollisionState_Uncollided) && (second_collidable->collision_state & CollisionState_Uncollided)))
            {
                if (NULL != s_CollisionEventCbPtr)
                {
                    first_collidable->collision_state  = CollisionState_Collided;
                    second_collidable->collision_state = CollisionState_Collided;
                    (*s_CollisionEventCbPtr)(first_collidable, second_collidable);
                }
            }
            else if (!collides && ((first_collidable->collision_state & CollisionState_Collided) && (second_collidable->collision_state & CollisionState_Collided)))
            {
                if (NULL != s_CollisionEventCbPtr)
                {
                    first_collidable->collision_state  = CollisionState_Uncollided;
                    second_collidable->collision_state = CollisionState_Uncollided;
                    (*s_CollisionEventCbPtr)(first_collidable, second_collidable);
                }
            }
        }
    }

    return 0;
}