#include "obj_registry.h"

static void** g_Registry = NULL;
static int    g_RegistryEntires = 32;
static int    g_Count = 0;

int register_obj(void* obj, int* handle)
{
	if (g_Count == g_RegistryEntires)
	{
		// TODO: Allocate more memory
	}

	if (NULL == g_Registry)
	{
		void** reg = malloc(g_RegistryEntires * sizeof * reg);
		if (NULL == reg)
		{
			PRINT_ERR("[obj_registry]: Failed to allocate sufficient memory for Registry ptr.");
			return;
		}

		g_Registry = reg;
	}

	g_Registry[g_Count] = obj;
	*handle = g_Count;
	g_Count++;

	return 0;
}

void* get_obj_from_registry(const int* handle)
{
	if (NULL == g_Registry)
	{
		PRINT_ERR("[obj_registry]: Registry was not initialized.");
		return;
	}
	if (*handle >= g_Count)
	{
		PRINT_ERR("[obj_registry]: Handle value is greater than registry entries count.");
		return;
	}

	return g_Registry[*handle];
}
