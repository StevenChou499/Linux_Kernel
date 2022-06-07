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

/**
 * @brief allocate memory to initialize queue
 * 
 * @param q pointer to the queue itself
 * @param s size of queue
 */
void queue_init(queue_t *q, size_t s)
{
    q->buffer = (uint8_t *)malloc(sizeof(uint8_t) * s);
    q->size = s;
    q->head = q->tail = 0;
}

/**
 * @brief free the allocated memory in queue
 * 
 * @param q pointer to the queue itself
 */
void queue_destroy(queue_t *q)
{
    free(q->buffer);
}

/**
 * @brief insert the elements in @buffer to queue
 * 
 * @param q pointer to the queue itself
 * @param buffer 
 * @param size 
 */
void queue_put(queue_t *q, uint8_t *buffer, size_t size)
{

}


#endif