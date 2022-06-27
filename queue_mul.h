#ifndef queue_mul_h_
#define queue_mul_h_

#include <pthread.h>
#include <stdint.h>

typedef struct {
    // backing buffer and size
    uint8_t *buffer;
    size_t size;

    // backing buffer's memfd descriptor
    int fd;

    // read / write indices
    size_t head, tail;

    // synchronization primitives
    pthread_cond_t readable, writeable;
    pthread_mutex_t lock;
    size_t **consumer_ptr;
    // uint32_t p_times, c_times;
} queue_t;

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/types.h>

/* Convenience wrappers for erroring out */
static inline void queue_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "queue error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}
static inline void queue_error_errno(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "queue error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, " (errno %d)\n", errno);
    va_end(args);
    abort();
}

/** Initialize a blocking queue *q* of size *s* */
void queue_init(queue_t *q, size_t s, size_t m)
{
    /* We mmap two adjacent pages (in virtual memory) that point to the same
     * physical memory. This lets us optimize memory access, so that we don't
     * need to even worry about wrapping our pointers around until we go
     * through the entire buffer.
     */

    size_t real_mmap_size;
    size_t real_mmap = 0;
    size_t buffer_algin = ((s - 1 + getpagesize()) / getpagesize()) * getpagesize();
    if(m * sizeof(size_t) * 2 > getpagesize())
        real_mmap = ((m * 2 * sizeof(size_t) - 1 + getpagesize()) / getpagesize()) * getpagesize();
    printf("real_mmap = %lu, buffer_algin = %lu\n", real_mmap, buffer_algin);

    real_mmap_size =  real_mmap > buffer_algin ? real_mmap : buffer_algin;

    // Check that the requested size is a multiple of a page. If it isn't, we're
    // in trouble.
    if (s % getpagesize() != 0) {
        fprintf(stderr, 
            "Requested size (%lu) is not a multiple of the page size (%d),\n", s,
            getpagesize());
        fprintf(stderr, "Changing to %lu bytes.\n", real_mmap_size);
    }

    // Create an anonymous file backed by memory
    if ((q->fd = memfd_create("queue_region", 0)) == -1)
        queue_error_errno("Could not obtain anonymous file");

    // Set buffer size
    if (ftruncate(q->fd, real_mmap_size) != 0)
        queue_error_errno("Could not set size of anonymous file");

    // Ask mmap for a good address
    if ((q->buffer = mmap(NULL, real_mmap_size * 2, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS,
                          -1, 0)) == MAP_FAILED)
        queue_error_errno("Could not allocate virtual memory");

    // Mmap first region
    if (mmap(q->buffer, real_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
             q->fd, 0) == MAP_FAILED)
        queue_error_errno("Could not map buffer into first virtual memory");

    // Mmap second region, with exact address
    if (mmap(q->buffer + real_mmap_size, real_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
             q->fd, 0) == MAP_FAILED)
        queue_error_errno("Could not map buffer into second virtual memory");

    // Initialize synchronization primitives
    if (pthread_mutex_init(&q->lock, NULL) != 0)
        queue_error_errno("Could not initialize mutex");
    if (pthread_cond_init(&q->readable, NULL) != 0)
        queue_error_errno("Could not initialize condition variable");
    if (pthread_cond_init(&q->writeable, NULL) != 0)
        queue_error_errno("Could not initialize condition variable");

    // Initialize remaining members
    q->size = real_mmap_size;
    q->head = q->tail = 0;
    // q->head = q->tail = q->p_times = q->c_times = 0;
    q->consumer_ptr = malloc(sizeof(size_t *));
    if(!q->consumer_ptr)
        fprintf(stderr, "couldn't allocate comsumer pointer\n");
}

/** Destroy the blocking queue *q* */
void queue_destroy(queue_t *q)
{
    if (munmap(q->buffer + q->size, q->size) != 0)
        queue_error_errno("Could not unmap buffer");

    if (munmap(q->buffer, q->size) != 0)
        queue_error_errno("Could not unmap buffer");

    if (close(q->fd) != 0)
        queue_error_errno("Could not close anonymous file");

    if (pthread_mutex_destroy(&q->lock) != 0)
        queue_error_errno("Could not destroy mutex");

    if (pthread_cond_destroy(&q->readable) != 0)
        queue_error_errno("Could not destroy condition variable");

    if (pthread_cond_destroy(&q->writeable) != 0)
        queue_error_errno("Could not destroy condition variable");
}

/** Insert into queue *q* a message of *size* bytes from *buffer*
 *
 * Blocks until sufficient space is available in the queue.
 */
void queue_put(queue_t *q, uint8_t **buffer, size_t size)
{
    pthread_mutex_lock(&q->lock);

    // Wait for space to become available
    while ((q->size - (q->tail - q->head)) < (size + sizeof(size_t))) {
        // q->p_times++;
        printf("publisher blocked...\n");
        pthread_cond_wait(&q->writeable, &q->lock);
    }

    // Write message
    printf("queue_put for size = %lu, q->head = %lu, q->tail = %lu\n", size, q->head, q->tail);
    memcpy(&q->buffer[q->tail], *buffer, size);
    // printf("%ld\n", (size_t) **(size_t **)buffer);
    

    // Increment write index and buffer index
    q->tail += size;
    *buffer += size;
    // printf("After queue_put for size = %lu, q->head = %lu, q->tail = %lu\n", size, q->head, q->tail);

    pthread_cond_signal(&q->readable);
    pthread_mutex_unlock(&q->lock);
}

/** Retrieves a message of at most *max* bytes from queue *q* and writes
 * it to *buffer*.
 *
 * Returns the number of bytes in the written message.
 */
size_t queue_get(queue_t *q, uint8_t **buffer, size_t size)
{
    pthread_mutex_lock(&q->lock);

    // Wait for a message that we can successfully consume to reach the front of
    // the queue
    while ((q->tail == q->head)) {
        // q->c_times++;
        printf("consumer blocked...\n");
        pthread_cond_wait(&q->readable, &q->lock);
    }
    
    // Read message body
    printf("queue_get for size = %lu, q->head = %lu, q->tail = %lu\n", size, q->head, q->tail);
    memcpy(*buffer, &q->buffer[q->head], size);

    // Consume the message by incrementing the read pointer
    q->head += size;
    *buffer += size;
    // printf("After queue_get for size = %lu, q->head = %lu, q->tail = %lu\n", size, q->head, q->tail);

    // When read buffer moves into 2nd memory region, we can reset to the 1st
    // region
    if (q->head >= q->size) {
        q->head -= q->size; q->tail -= q->size;
    }

    pthread_cond_signal(&q->writeable);
    pthread_mutex_unlock(&q->lock);

    return size;
}

#endif