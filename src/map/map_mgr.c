#include "map/map_mgr.h"
#include "map/initial_map.h"
#include "utils.h"

static int              s_CurrMapIdx = 0;
static int              s_NextAvailableIdx = 0;
static int              s_MapFunctionsCapacity = 32;
static MapFuncsDef**    s_MapFunctions;

// ! Allocates memory on heap !
static int init_map_functions()
{
	s_MapFunctionsCapacity *= 2;
	MapFuncsDef** map_funcs_ptr = realloc(s_MapFunctions, s_MapFunctionsCapacity * sizeof *map_funcs_ptr);
	if (NULL == map_funcs_ptr)
	{
		PRINT_ERR("[map_mgr]: Failed to allocate sufficient memory for MapFuncsDef arr.");
		return TERMINATE_ERR_CODE;
	}

	s_MapFunctions = map_funcs_ptr;
	
	return 0;
}

static const MapFuncsDef* get_curr_map()
{
	MapFuncsDef* map_def = s_MapFunctions[s_CurrMapIdx];
	if (NULL == map_def)
	{
		PRINT_ERR("[map_mgr]: Failed to load current map funcs def.");
		return NULL;
	}

	return map_def;
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

int map_mgr_init()
{
	if (NULL == s_MapFunctions)
	{
		int map_arr_alloc_res = init_map_functions();
		if (TERMINATE_ERR_CODE == map_arr_alloc_res)
		{
			PRINT_ERR("[map_mgr]: Failed to register a new map (map func arr allocation failed).");
			return TERMINATE_ERR_CODE;
		}
	}

	initial_map_init();

	return 0;
}

int map_mgr_register_map(const MapFuncsDef* map_funcs_def)
{
	if (s_NextAvailableIdx >= s_MapFunctionsCapacity)
	{
		int map_arr_alloc_res = init_map_functions();
		if (TERMINATE_ERR_CODE == map_arr_alloc_res)
		{
			PRINT_ERR("[map_mgr]: Failed to register a new map (map func arr allocation failed).");
			return TERMINATE_ERR_CODE;
		}
	}

	s_MapFunctions[s_NextAvailableIdx] = map_funcs_def;
	s_NextAvailableIdx++;

	return 0;
}

int map_mgr_load_map()
{
	const MapFuncsDef* map_def = get_curr_map();
	if (NULL == map_def)
	{
		PRINT_ERR("[map_mgr]: Failed to get current map ptr.");
		return TERMINATE_ERR_CODE;
	}

	map_def->add_background();
	map_def->add_path();

	return 0;
}

int map_mgr_advance_to_next_map()
{
	// TODO: Do we need to clear prev map resources?
	s_CurrMapIdx++;
	// TODO: If we've reached the end - return some err code
	return 0;
}

const PathSegment** map_mgr_get_path()
{
	const MapFuncsDef* map_def = get_curr_map();
	if (NULL == map_def)
	{
		PRINT_ERR("[map_mgr]: Failed to get current map ptr.");
		return NULL;
	}

	return map_def->get_path();
}

int map_mgr_get_path_len()
{
	const MapFuncsDef* map_def = get_curr_map();
	if (NULL == map_def)
	{
		PRINT_ERR("[map_mgr]: Failed to get current map ptr.");
		return TERMINATE_ERR_CODE;
	}

	return map_def->get_path_len();
}

int map_mgr_free_resources()
{
	if (NULL == s_MapFunctions)
	{
		return 0;
	}

	for (int i = 0; i < s_NextAvailableIdx; i++)
	{
		free(s_MapFunctions[i]);
	}

	free(s_MapFunctions);

	return 0;
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
