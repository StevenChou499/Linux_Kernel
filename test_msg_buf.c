#include <stdio.h>                         
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "queue_msg_buf.h"

#define BUFFER_SIZE (getpagesize())
#define NUM_THREADS (1)
#define MESSAGES_PER_THREAD (getpagesize() * 2)
#define SIZE_OF_MESSAGE 100ULL

uint32_t comp(const void *elem1, const void *elem2)
{
    uint32_t f = *((uint32_t *) elem1);
    uint32_t s = *((uint32_t *) elem2);
    if (f > s)
        return 1;
    if (f < s)
        return -1;
    return 0;
}

typedef struct {
    queue_t q;
    uint32_t messages_per_thread;
    uint32_t num_threads;
} rbuf_t;

size_t in[65536];
size_t out[65536];

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

static void *publisher_loop(void *arg)
{
    rbuf_t *r = (rbuf_t *) arg;
    size_t i;
    size_t **publisher_ptr = malloc(sizeof(size_t *));
    *publisher_ptr = in;
    size_t full_put = (r->messages_per_thread * r->num_threads) / SIZE_OF_MESSAGE;
    size_t remain_put = (r->messages_per_thread * r->num_threads) % SIZE_OF_MESSAGE;
    for (i = 0; i < full_put; i++)
        queue_put(&r->q, (uint8_t **) publisher_ptr, sizeof(size_t) * SIZE_OF_MESSAGE);
    if(remain_put)
        queue_put(&r->q, (uint8_t **) publisher_ptr, sizeof(size_t) * remain_put);
    return (void *) (i * SIZE_OF_MESSAGE + remain_put);
}

static void *consumer_loop(void *arg)
{
    rbuf_t *r = (rbuf_t *) arg;
    size_t i;
    size_t **consumer_ptr = malloc(sizeof(size_t *));
    *consumer_ptr = out;
    size_t full_get = (r->messages_per_thread) / SIZE_OF_MESSAGE;
    size_t remain_get = (r->messages_per_thread) % SIZE_OF_MESSAGE;
    for (i = 0; i < full_get; i++)
        queue_get(&r->q, (uint8_t **) consumer_ptr, sizeof(size_t) * SIZE_OF_MESSAGE);
    if(remain_get)
        queue_get(&r->q, (uint8_t **) consumer_ptr, sizeof(size_t) * remain_get);
    return (void *) (i * SIZE_OF_MESSAGE + remain_get);
}

int main(int argc, char *argv[])
{
    for(size_t i = 0; i < 65536ULL; i++) {
        in[i] = i;
        out[i] = 0ULL;
    }

    rbuf_t r;
    r.num_threads = 1U;
    if (argc > 1)
        r.messages_per_thread = (uint32_t) atoi(argv[1]);
    else
        r.messages_per_thread = 65536;
    
    queue_init(&r.q, BUFFER_SIZE);

    pthread_t publisher_th;
    pthread_t consumer_th;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&publisher_th, &attr, &publisher_loop, (void *) &r);

    pthread_create(&consumer_th, &attr, &consumer_loop, (void *) &r);

    intptr_t sent;
    pthread_join(publisher_th, (void **) &sent);

    intptr_t recd;
    pthread_join(consumer_th, (void **) &recd);

    // for(size_t i = 0ULL; i < r.messages_per_thread; i++)
        // printf("%lu\n", out[i]);

    // printf("\npublisher sent %ld messages\n", sent);
    // printf("consumer received %ld messages\n", recd);

    pthread_attr_destroy(&attr);

    queue_destroy(&r.q);

    for(size_t i = 0; i < 65536; i++)
        printf("%ld\n", out[i]);

    return 0;
}