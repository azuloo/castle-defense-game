#ifndef _OBJ_REGISTRY_H
#define _OBJ_REGISTRY_H

#include "utils.h"

#define REGISTER_OBJ(type, handle)  register_obj((void*)type, handle);
#define GET_FROM_REGISTRY(handle)   get_obj_from_registry(handle);

int register_obj(void* obj, int* handle);
void* get_obj_from_registry(const int* handle);

#endif // _OBJ_REGISTRY_H
