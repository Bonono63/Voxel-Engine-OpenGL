#include <stdio.h>
#include <io.h>
#include <sys/stat.h>

int read_file(const char *path, char **out)
{
    FILE *file;

    struct stat st;
    stat(path, &st);
    size_t file_size = st.st_size;
    printf("file size: %zu\n",file_size);

    file = fopen(path, "rb");
    if (!file)
    {
        printf("Unable to open file at %s .\n",path);
        return -1;
    }
    
    *out = (char*) calloc(1,file_size);
    for (size_t i = 0 ; i < file_size ; i++)
    {
        char c = fgetc(file);
        //printf("%i %c %i\n", i, c, c);
        *(*out+i) = c;
    }

    fclose(file);
    return 0;
}
