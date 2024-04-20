#include "utils.h"

#include <stdio.h>
#include <string.h>

void str_concat(const char* s1, const char* s2, char** buf, size_t len)
{
	snprintf(buf, sizeof(buf) * len, "%s%s", s1, s2);
}

void print_err(char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(err_msg_buffer, ERR_MSG_BUFFER_LEN, format, args);
	PRINT_ERR(err_msg_buffer);
	memset(err_msg_buffer, 0, sizeof(err_msg_buffer));
	va_end(args);
}
