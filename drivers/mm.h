#ifndef MM_H
#define MM_H

#include <stddef.h>

void mm_init(void* start, size_t size);
void* kmalloc(size_t size);

#endif
