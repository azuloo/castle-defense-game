#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define SOURCE_ROOT _SOURCE_ROOT

#define STRINGINIZE(x) #x
#define STRVAL(x) STRINGINIZE(x)

#define TERMINATE_SUCCESS_CODE EXIT_SUCCESS
#define TERMINATE_ERR_CODE EXIT_FAILURE

#define APP_EXIT(code) exit(code)

#define ERR_MSG_BUFFER_LEN 1024
static char err_msg_buffer[ERR_MSG_BUFFER_LEN];

void print_err(const char* format, ...);
void str_concat(const char* s1, const char* s2, char** buf, size_t len);

#define PRINT_ERR(format, ...) fprintf(stderr, "ERR: %s:%d: %s\n", __FILE__, __LINE__, format)
#define PRINT_ERR_VARGS(format, ...) print_err(format, __VA_ARGS__)

#define CHECK_EXPR_FAIL_RET_TERMINATE(expr, format, ...) \
	if (!(expr)) { \
		PRINT_ERR_VARGS(format, __VA_ARGS__); \
		return TERMINATE_ERR_CODE; \
	}
#define CHECK_EXPR_FAIL_RET_NULL(expr, format, ...) \
	if (!(expr)) { \
		PRINT_ERR_VARGS(format, __VA_ARGS__); \
		return NULL; \
	}
#define CHECK_EXPR_FAIL_RET(expr, format, ...) \
	if (!(expr)) { \
		PRINT_ERR_VARGS(format, __VA_ARGS__); \
		return; \
	}

#endif // _UTILS_H
