#ifndef _MAP_INITIAL_H
#define _MAP_INITIAL_H

#include "entity.h"

int initial_add_background();
int initial_add_path();
void initial_free_resources();

const PathDef* get_initial_path();
int get_initial_path_len();

int initial_map_init();

#endif // _MAP_INITIAL_H
