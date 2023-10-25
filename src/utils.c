#include "utils.h"

#include <string.h>

char* str_concat(const char* s1, const char* s2)
{
	char* result = malloc(strlen(s1) + strlen(s2) + 1);
	if (NULL == result)
	{
		exit(TERMINATE_ERR_CODE);
	}

	strcpy(result, s1);
	strcat(result, s2);
	return result;
}
