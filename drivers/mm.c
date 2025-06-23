#include "mm.h"
#include <stdint.h>

static uint8_t* heap_start;
static uint8_t* heap_end;
static uint8_t* heap_curr;

void mm_init(void* start, size_t size) {
    heap_start = (uint8_t*)start;
    heap_end = heap_start + size;
    heap_curr = heap_start;
}

void* kmalloc(size_t size) {
    if (heap_curr + size > heap_end) {
        return 0; // out of memory
    }
    void* ptr = heap_curr;
    heap_curr += size;
    return ptr;
}
