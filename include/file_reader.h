#ifndef _FILE_READER_H
#define _FILE_READER_H

#include <stdio.h>

#define CHUNK_SIZE 2097152 // 2 MiB

typedef enum _ReadOpResult {
	READ_OK          = 0,
	READ_INVALID     = -1,
	READ_ERR         = -2,
	READ_OVERFLOW    = -3,
	READ_OUTOFMEM    = -4
} ReadOpResult;

extern int readall(char* path, char** data_ptr, size_t* size_ptr);

#endif // _FILE_READER_H