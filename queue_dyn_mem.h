#ifndef queue_dyn_mem_h_
#define queue_dyn_mem_h_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    // representing buffer and its size
    uint8_t *buffer;
    size_t size;

    // representing read and write indices
    size_t head, tail;

    // synchronization primitives
    pthread_cond_t readable, writeable;
    pthread_mutex_t lock;
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

    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->readable, NULL);
    pthread_cond_init(&q->writeable, NULL);
}

/**
 * @brief free the allocated memory in queue
 * 
 * @param q pointer to the queue itself
 */
void queue_destroy(queue_t *q)
{
    free(q->buffer);
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->readable);
    pthread_cond_destroy(&q->writeable);
}

/**
 * @brief insert the elements in @buffer to queue
 * 
 * @param q pointer to the queue itself
 * @param buffer pointer to the buffer inserting to queue
 * @param size size of the buffer
 */
void queue_put(queue_t *q, uint8_t *buffer, size_t size)
{
    pthread_mutex_lock(&q->lock);
    while ((q->size - (q->tail - q->head)) < (size + sizeof(size_t)))
        pthread_cond_wait(&q->writeable, &q->lock);

    memcpy(&q->buffer[q->tail], buffer, sizeof(size_t));
    q->tail += size;
    q->tail %= q->size;

    pthread_cond_signal(&q->readable);
    pthread_mutex_unlock(&q->lock);
}

size_t queue_get(queue_t *q, uint8_t *buffer, size_t max)
{
    pthread_mutex_lock(&q->lock);
    while ((q->tail - q->head) == 0)
        pthread_cond_wait(&q->readable, &q->lock);

    memcpy(buffer, &q->buffer[q->head], sizeof(size_t));
    q->head += max;
    q->head %= q->size;

    pthread_cond_signal(&q->writeable);
    pthread_mutex_unlock(&q->lock);

    return max;
}

#endif