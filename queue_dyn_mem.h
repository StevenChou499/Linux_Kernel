#ifndef queue_dyn_mem_h_
#define queue_dyn_mem_h_

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    // representing buffer and its size
    uint8_t *buffer;
    size_t size;

    // representing read and write indices
    size_t head, tail;
} queue_t;

#endif