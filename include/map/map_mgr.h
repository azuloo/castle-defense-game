#ifndef _MAP_MGR_H
#define _MAP_MGR_H

#include "entity.h"

typedef struct MapFuncsDef
{
	int                    (*add_background)();
	int                    (*add_path)();
	void                   (*free_resources)();

	const PathSegment**    (*get_path)();
	int                    (*get_path_len)();
} MapFuncsDef;

int map_mgr_register_map(const MapFuncsDef* map_funcs_def);
int map_mgr_load_map();
int map_mgr_advance_to_next_map();

const PathSegment**   map_mgr_get_path();
int                   map_mgr_get_path_len();

int map_mgr_init();
int map_mgr_free_resources();

#endif // _MAP_MGR_H
