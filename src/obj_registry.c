#include "obj_registry.h"
#include "utils.h"

static void**   s_Registry = NULL;
static int      s_RegistryCapacity = 32;
static int      s_RegistryObjCount = 0;

// ! Allocates memory on heap !
static int alloc_registry()
{
	s_RegistryCapacity *= 2;
	void** reg = realloc(s_Registry, s_RegistryCapacity * sizeof *reg);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != reg, "[obj_registry]: Failed to allocate sufficient memory for Registry ptr.");

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
		CHECK_EXPR_FAIL_RET_TERMINATE(TERMINATE_ERR_CODE != alloc_registry_res, "[obj_registry]: Failed to create registry container.");
	}

	s_Registry[s_RegistryObjCount] = obj;
	*handle = s_RegistryObjCount;
	s_RegistryObjCount++;

	return 0;
}

void* get_obj_from_registry(int handle)
{
	CHECK_EXPR_FAIL_RET_NULL(NULL != s_Registry, "[obj_registry]: Registry was not initialized.");
	CHECK_EXPR_FAIL_RET_NULL(handle < s_RegistryObjCount || handle >= s_RegistryObjCount, "[obj_registry]: Handle value is out of bounds.");

	return s_Registry[handle];
}

// ----------------------- PUBLIC FUNCTIONS END ----------------------- //
