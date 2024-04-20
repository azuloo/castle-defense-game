#ifndef _GLOBAL_DECL_H
#define _GLOBAL_DECL_H

// Graphics
typedef struct TransformDef TransformDef;
typedef struct DrawableDef DrawableDef;
typedef struct GLFWwindow GLFWwindow;
typedef struct DrawBufferData DrawBufferData;

// Entity
typedef enum EntityType EntityType;
typedef struct EntityDef EntityDef;
typedef struct EnemyWaveDef EnemyWaveDef;
typedef struct EnemyWaveCnf EnemyWaveCnf;

// Physics
typedef struct CollisionBox2D CollisionBox2D;
typedef struct Collidable2D Collidable2D;

// Map manager
typedef struct PathSegment PathSegment;
typedef struct PathDef PathDef;

#endif // _GLOBAL_DECL_H