#include <sys/stat.h>

/*
 * Linux implementation to read a file
 * relies of POSIX standards to read file size
 * */
int read_file(const char * path, char** out);
