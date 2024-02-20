#include "file_reader.h"
#include "utils.h"
#include "stb_image.h"

#include <stdlib.h>

static unsigned char* load_image(const char* path, int* width, int* height, int* nr_channels)
{
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path, width, height, nr_channels, 0);
    return data;
}

// ! Allocates memory on heap !
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
    if (NULL == in || ferror(in))
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

void get_file_path(const char* name, char** buf, size_t len)
{
    str_concat(STRVAL(SOURCE_ROOT), name, buf, len);
}

int fr_read_image_data(const char* path, unsigned char** data, int* width, int* height)
{
    int nr_channels;
    // TODO: Pass nr_channels as arg when needed
    *data = load_image(path, width, height, &nr_channels);
    if (NULL == data)
    {
        PRINT_ERR("[file_reader]: Failed to load image.");
        return TERMINATE_ERR_CODE;
    }

    return 0;
}
