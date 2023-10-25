#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>

#define SOURCE_ROOT _SOURCE_ROOT

#define STRINGINIZE(x) #x
#define STRVAL(x) STRINGINIZE(x)

#define TERMINATE_SUCCESS_CODE EXIT_SUCCESS
#define TERMINATE_ERR_CODE EXIT_FAILURE

extern char* str_concat(const char* s1, const char* s2);

#endif // _UTILS_H
