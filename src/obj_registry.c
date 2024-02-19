#include "obj_registry.h"

static void**   s_Registry = NULL;
static int      s_RegistryCapacity = 32;
static int      s_RegistryObjCount = 0;

// ! Allocates memory on heap !
static int alloc_registry()
{
	s_RegistryCapacity *= 2;
	void** reg = realloc(s_Registry, s_RegistryCapacity * sizeof *reg);
	if (NULL == reg)
	{
		PRINT_ERR("[obj_registry]: Failed to allocate sufficient memory for Registry ptr.");
		return TERMINATE_ERR_CODE;
	}

	s_Registry = reg;
	
	return 0;
}

// ----------------------- PUBLIC FUNCTIONS ----------------------- //

void registry_free()
{
	if (NULL == s_Registry)
	{
		return;
	}

	free(s_Registry);
}

int register_obj(void* obj, int* handle)
{
	if (NULL == s_Registry || s_RegistryObjCount == s_RegistryCapacity)
	{
		int alloc_registry_res = alloc_registry();
		if (TERMINATE_ERR_CODE == alloc_registry_res)
		{
			return TERMINATE_ERR_CODE;
		}
	}

	s_Registry[s_RegistryObjCount] = obj;
	*handle = s_RegistryObjCount;
	s_RegistryObjCount++;

	return 0;
}

void* get_obj_from_registry(const int* handle)
{
	if (NULL == s_Registry)
	{
		PRINT_ERR("[obj_registry]: Registry was not initialized.");
		return NULL;
	}
	if (*handle >= s_RegistryObjCount)
	{
		PRINT_ERR("[obj_registry]: Handle value is greater than registry entries count.");
		return NULL;
	}

	return s_Registry[*handle];
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
