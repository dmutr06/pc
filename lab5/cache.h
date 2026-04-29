#ifndef CACHE_H
#define CACHE_H

#define MAX_CACHED_FILES 100

typedef struct {
    char filepath[256];
    int fd;
    int file_size;
    const char *content_type;
} FileCache;

FileCache* get_file_cache(const char *filepath);

#endif // CACHE_H
