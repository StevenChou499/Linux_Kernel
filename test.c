#include <stdio.h>                         
#include <unistd.h>
#include <time.h>

#include "queue.h"

#define BUFFER_SIZE (getpagesize())
#define NUM_THREADS (8)
#define MESSAGES_PER_THREAD (getpagesize() * 2)

/*struct target {
    pthread_t thread;
    uint8_t buffer[8192];
};*/

/**
 * @brief Get timestamp
 * @return timestamp now
 */
uint64_t get_time()
{
    struct timespec ts;
    clock_gettime(0, &ts);
    return (uint64_t)(ts.tv_sec * 1e6 + ts.tv_nsec / 1e3);
}

static void *consumer_loop(void *arg)
{
    queue_t *q = (queue_t *) arg;
    size_t count = 0;
    for (size_t i = 0; i < MESSAGES_PER_THREAD; i++) {
        size_t x;
        queue_get(q, (uint8_t *) &x, sizeof(size_t));
        printf("%ld ", x);
        count++;
    }
    return (void *) count;
}

static void *publisher_loop(void *arg)
{
    queue_t *q = (queue_t *) arg;
    size_t i;
    for (i = 0; i < NUM_THREADS * MESSAGES_PER_THREAD; i++)
        queue_put(q, (uint8_t *) &i, sizeof(size_t));
    return (void *) i;
}

/**
 * @brief Prints a buffer to a file
 * @param f file descriptor
 * @param buf Pointer to the buffer
 * @param size Size of buffer
 */
void print_buf_to_file(FILE *f, uint8_t *buf, size_t size)
{
    for(size_t i = 0; i < size; i += 3){
        if(i % 900 == 0)
            fprintf(f, "\n");
        
        fprintf(f, buf[i]);
    }
}

int main(int argc, char *argv[])
{
    FILE *in = fopen("in.txt", "w");
    FILE *out = fopen("out.txt", "w");

    queue_t q;
    queue_init(&q, BUFFER_SIZE);

    uint64_t start = get_time();

    pthread_t publisher;
    pthread_t consumers[NUM_THREADS];

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&publisher, &attr, &publisher_loop, (void *) &q);

    for (intptr_t i = 0; i < NUM_THREADS; i++)
        pthread_create(&consumers[i], &attr, &consumer_loop, (void *) &q);

    intptr_t sent;
    pthread_join(publisher, (void **) &sent);
    printf("publisher sent %ld messages\n", sent);

    intptr_t recd[NUM_THREADS];
    for (intptr_t i = 0; i < NUM_THREADS; i++) {
        pthread_join(consumers[i], (void **) &recd[i]);
        printf("consumer %ld received %ld messages\n", i, recd[i]);
    }

    uint64_t end = get_time();

    printf("Total runtime : %ldms\n", end - start);
    
    // print_buf_to_file(in, )


    pthread_attr_destroy(&attr);

    queue_destroy(&q);

    return 0;
}