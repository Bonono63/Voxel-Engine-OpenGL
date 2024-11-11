#include <stdio.h>
#include <stdlib.h>
#include <io.h>

int read_file(const char *path, char **out)
{
    FILE *file;

    struct stat st;
    // sys call to retrieve file size on linux
    if (stat(path, &st))
    {
        printf("[IO] File %s does not exist.\n", path);
        return -1;
    }
    
    size_t file_size = st.st_size;

    file = fopen(path, "rb");
    if (!file)
    {
        printf("[IO] Unable to retrieve size of file %s .\n",path);
        return -1;
    }
    
    *out = (char*) calloc(1,file_size);

    size_t read_size = fread(*out, sizeof(char), file_size, file);

    if (read_size == file_size)
    {
        printf("[IO] Read %s\n", path);
    }
    else
    {
        printf("[IO] Error reading file | size/error: %zu\n", file_size);
    }

    fclose(file);
    return 0;
}
