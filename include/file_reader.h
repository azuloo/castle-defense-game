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

int readall(char* path, char** data_ptr, size_t* size_ptr);
void get_file_path(const char* name, char** buf, size_t len);
int fr_read_image_data(const char* path, unsigned char** data, int* width, int* height);
int fr_free_image_resources(unsigned char* img_data);

#endif // _FILE_READER_H
