#include <stdio.h>                         
#include <unistd.h>
#include <time.h>

#include "queue.h"

#define BUFFER_SIZE (getpagesize())
#define NUM_THREADS (8)
#define MESSAGES_PER_THREAD (getpagesize() * 2)

typedef struct ringbuf {
    queue_t q;
    size_t msg_per_thd;
    size_t num_of_thd;
    // size_t buf_size;
} ringbuf_t;

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
    ringbuf_t *r = (ringbuf_t *) arg;
    size_t count = 0;
    size_t message_per_thread = r->msg_per_thd;
    for (size_t i = 0; i < message_per_thread; i++) {
        size_t x;
        queue_get(&r->q, (uint8_t *) &x, sizeof(size_t));
        // printf("%ld ", x);
        count++;
    }
    return (void *) count;
}

static void *publisher_loop(void *arg)
{
    ringbuf_t *r = (ringbuf_t *) arg;
    size_t i;
    size_t message_per_thread = r->msg_per_thd;
    size_t num_threads = r->num_of_thd;
    for (i = 0; i < num_threads * message_per_thread; i++)
        queue_put(&r->q, (uint8_t *) &i, sizeof(size_t));
    return (void *) i;
}

int main(int argc, char *argv[])
{
    // queue_t q;
    ringbuf_t r;
    queue_init(&r.q, BUFFER_SIZE);
    r.num_of_thd = 1;
    r.msg_per_thd = MESSAGES_PER_THREAD;

    uint64_t start = get_time();

    pthread_t publisher;
    pthread_t consumers[r.num_of_thd];

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&publisher, &attr, &publisher_loop, (void *) &r);

    for (intptr_t i = 0; i < r.num_of_thd; i++)
        pthread_create(&consumers[i], &attr, &consumer_loop, (void *) &r);

    intptr_t sent;
    pthread_join(publisher, (void **) &sent);

    intptr_t recd[r.num_of_thd];
    for (intptr_t i = 0; i < r.num_of_thd; i++)
        pthread_join(consumers[i], (void **) &recd[i]);

    uint64_t end = get_time();
    printf("\npublisher sent %ld messages\n", sent);
    for (intptr_t i = 0; i < r.num_of_thd; i++) {
        printf("consumer %ld received %ld messages\n", i, recd[i]);
    }

    printf("Total runtime : %ldus\n", end - start);

    pthread_attr_destroy(&attr);

    queue_destroy(&r.q);

    return 0;
}