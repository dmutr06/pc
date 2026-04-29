#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "cache.h"

FileCache cache[MAX_CACHED_FILES];
int cache_size = 0;
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

FileCache* get_file_cache(const char *filepath) {
    pthread_mutex_lock(&cache_mutex);
    
    for (int i = 0; i < cache_size; i++) {
        if (strcmp(cache[i].filepath, filepath) == 0) {
            pthread_mutex_unlock(&cache_mutex);
            return &cache[i];
        }
    }
    
    if (cache_size >= MAX_CACHED_FILES) {
        pthread_mutex_unlock(&cache_mutex);
        return NULL;
    }
    
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        pthread_mutex_unlock(&cache_mutex);
        return NULL;
    }
    
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0 || S_ISDIR(file_stat.st_mode)) {
        close(fd);
        pthread_mutex_unlock(&cache_mutex);
        return NULL;
    }
    
    FileCache *new_cache = &cache[cache_size++];
    strncpy(new_cache->filepath, filepath, sizeof(new_cache->filepath) - 1);
    new_cache->fd = fd;
    new_cache->file_size = file_stat.st_size;
    
    const char *content_type = "text/plain";
    if (strstr(filepath, ".html")) content_type = "text/html";
    else if (strstr(filepath, ".css")) content_type = "text/css";
    else if (strstr(filepath, ".js")) content_type = "application/javascript";
    else if (strstr(filepath, ".png")) content_type = "image/png";
    else if (strstr(filepath, ".jpg") || strstr(filepath, ".jpeg")) content_type = "image/jpeg";
    
    new_cache->content_type = content_type;
    
    pthread_mutex_unlock(&cache_mutex);
    return new_cache;
}
