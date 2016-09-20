#ifndef __MEMORY_UTILS_H__
#define __MEMORY_UTILS_H__

#include <string.h>

struct memstruct {
    char *memory;
    size_t size;
};

struct memstruct albumstr;

void memory_init();
void memory_clean();
size_t memory_write(void *contents, size_t size, size_t nmemb, void *userp);

#endif //__MEMORY_UTILS_H__
