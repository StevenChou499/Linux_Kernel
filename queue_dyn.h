#ifndef queue_dyn_h_
#define queue_dyn_h_

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
    // allocate a specific amount of memory for ring buffer
    q->buffer = (uint8_t *)malloc(sizeof(uint8_t) * s);
    if(!q->buffer) {
        fprintf(stderr, "queue error: Couldn't allocate memory!\n");
        abort();
    }

    // Initialize synchronization primitives
    if (pthread_mutex_init(&q->lock, NULL) != 0)
        fprintf(stderr, "Could not initialize mutex");
    if (pthread_cond_init(&q->readable, NULL) != 0)
        fprintf(stderr, "Could not initialize condition variable");
    if (pthread_cond_init(&q->writeable, NULL) != 0)
        fprintf(stderr, "Could not initialize condition variable");
    
    // Initiallize buffer size and indices
    q->size = s;
    q->head = q->tail = 0;
}

/**
 * @brief free the allocated memory in queue
 * and destroy mutex and condition variables
 * 
 * @param q pointer to the queue itself
 */
void queue_destroy(queue_t *q)
{
    free(q->buffer);
    if (pthread_mutex_destroy(&q->lock) != 0)
        fprintf(stderr, "Could not destroy mutex");

    if (pthread_cond_destroy(&q->readable) != 0)
        fprintf(stderr, "Could not destroy condition variable");

    if (pthread_cond_destroy(&q->writeable) != 0)
        fprintf(stderr, "Could not destroy condition variable");
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

    while ((q->tail + sizeof(size)) % q->size == q->head)
        pthread_cond_wait(&q->writeable, &q->lock);

    memcpy(&q->buffer[q->tail], buffer, sizeof(size_t));
     // printf("put : %ld\n", *(size_t *) buffer);
    q->tail += size;
    q->tail %= q->size;

    pthread_cond_signal(&q->readable);
    pthread_mutex_unlock(&q->lock);
}

/**
 * @brief duplicate the content into the ring buffer
 * 
 * @param q pointer to the queue itself
 * @param buffer pointer to the duplicated content
 * @param max the size of duplicated content
 * @return size_t 
 */
size_t queue_get(queue_t *q, uint8_t *buffer, size_t max)
{
    pthread_mutex_lock(&q->lock);

    while ((q->tail - q->head) == 0)
        pthread_cond_wait(&q->readable, &q->lock);

    memcpy(buffer, &q->buffer[q->head], sizeof(size_t));
    // printf("get : %ld\n", *(size_t *) buffer);
    q->head += max;
    q->head %= q->size;

    pthread_cond_signal(&q->writeable);
    pthread_mutex_unlock(&q->lock);

    return max;
}

#endif