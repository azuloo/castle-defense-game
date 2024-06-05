#ifndef _MAP_MGR_H
#define _MAP_MGR_H

#include "lin_alg.h"
#include "global_decl.h"

typedef struct CastleDef
{
	enum EntityType    type;
	int                drawable_handle;
	Collidable2D*      collidable2D;
} CastleDef;

typedef struct PathSegment
{
	Vec2 start;
	Vec2 end;
} PathSegment;

typedef struct PathDef
{                
	PathSegment        path_segment;
	Collidable2D*      collidable2D;
	int                drawable_handle;
} PathDef;

typedef struct MapFuncsDef
{
	int                    (*map_init)();
	int                    (*add_background)();
	int                    (*add_path)();
	Vec2                   (*get_init_direction)();
	int                    (*add_castle)();
	int                    (*get_castle)(CastleDef**);
	int                    (*on_window_resize)();
	Vec2                   (*get_path_start)();
	void                   (*free_resources)();

	const PathDef*         (*get_path)();
	int                    (*get_path_len)();
} MapFuncsDef;

int map_mgr_register_map(const MapFuncsDef* map_funcs_def);
int map_mgr_load_map();
int map_mgr_advance_to_next_map();
int map_mgr_on_window_resize();
Vec2 map_mgr_get_init_direction();

const PathDef* map_mgr_get_path();
Vec2 map_mgr_get_path_start();
int map_mgr_get_path_len();
int map_mgr_get_castle(CastleDef** dest);

int map_mgr_init();
int map_mgr_free_resources();

#endif // _MAP_MGR_H
