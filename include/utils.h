#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define SOURCE_ROOT _SOURCE_ROOT

#define STRINGINIZE(x) #x
#define STRVAL(x) STRINGINIZE(x)

#define TERMINATE_SUCCESS_CODE EXIT_SUCCESS
#define TERMINATE_ERR_CODE EXIT_FAILURE

#define APP_EXIT(code) exit(code)

static char err_msg_buffer[1024];

#define PRINT_ERR(msg) fprintf(stderr, "ERR: %s:%d: %s\n", __FILE__, __LINE__, msg)
// TODO: Is there a better way to do this?
#define PRINT_ERR_VARGS(msg, ...) \
	sprintf_s(err_msg_buffer, msg, __VA_ARGS__); \
	PRINT_ERR(err_msg_buffer); \
	memset(err_msg_buffer, 0, sizeof(err_msg_buffer));

void str_concat(const char* s1, const char* s2, char** buf, size_t len);

#endif // _UTILS_H
