#include "file_reader.h"

#include <stdlib.h>

int readall(char* path, char** data_ptr, size_t* size_ptr)
{
    if (NULL == path || NULL == data_ptr || NULL == size_ptr)
    {
        return READ_INVALID;
    }

    char* data = NULL, * temp;
    size_t size = 0;
    size_t used = 0;
    size_t n;

    FILE* in = fopen(path, "rb");
    if (ferror(in))
    {
        return READ_ERR;
    }

    while (1)
    {
        if (used + CHUNK_SIZE + 1 > size)
        {
            size = used + CHUNK_SIZE + 1;
            if (size <= used)
            {
                free(data);
                return READ_OVERFLOW;
            }

            temp = realloc(data, size);
            if (temp == NULL)
            {
                free(data);
                return READ_OUTOFMEM;
            }

            data = temp;
        }

        n = fread(data + used, 1, CHUNK_SIZE, in);
        if (n == 0)
        {
            break;
        }

        used += n;
    }

    if (ferror(in))
    {
        free(data);
        return READ_ERR;
    }

    temp = realloc(data, used + 1);
    if (temp == NULL)
    {
        free(data);
        return READ_OUTOFMEM;
    }

    data = temp;
    data[used] = '\0';

    *data_ptr = data;
    *size_ptr = used;

    fclose(in);

    return READ_OK;
}
