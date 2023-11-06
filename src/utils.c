#include "utils.h"

#include <stdio.h>
#include <string.h>

char* str_concat(const char* s1, const char* s2, char** buf, size_t len)
{
	snprintf(buf, sizeof(buf) * len, "%s%s", s1, s2);
}
