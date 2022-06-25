#include <stdio.h>                         
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "queue_con.h"

#define BUFFER_SIZE (getpagesize())
#define NUM_THREADS (2)
#define MESSAGES_PER_THREAD (getpagesize() * 2)
#define SIZE_OF_MESSAGE 500ULL

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
    // printf("full_put = %lu\n", full_put);
    for (i = 0; i < full_put; i++) {
        // printf("pub %ld time\n", i);
        queue_put(&r->q, (uint8_t **) publisher_ptr, sizeof(size_t) * SIZE_OF_MESSAGE);
    }
    // printf("pub %ld time\n", full_put);
    if(remain_put)
        queue_put(&r->q, (uint8_t **) publisher_ptr, sizeof(size_t) * remain_put);
    return (void *) (i * SIZE_OF_MESSAGE + remain_put);
}

static void *consumer_loop(void *arg)
{
    rbuf_t *r = (rbuf_t *) arg;
    size_t i;
    // size_t **consumer_ptr = malloc(sizeof(size_t *));
    // *consumer_ptr = out;
    size_t full_get = (r->messages_per_thread) / SIZE_OF_MESSAGE;
    size_t remain_get = (r->messages_per_thread) % SIZE_OF_MESSAGE;
    for (i = 0; i < full_get; i++) {
        // printf("con %ld time\n", i);
        queue_get(&r->q, (uint8_t **) r->q.consumer_ptr, sizeof(size_t) * SIZE_OF_MESSAGE);
    }
    // printf("con %ld time\n", full_get);
    if(remain_get)
        queue_get(&r->q, (uint8_t **) r->q.consumer_ptr, sizeof(size_t) * remain_get);
    return (void *) (i * SIZE_OF_MESSAGE + remain_get);
}

int main(int argc, char *argv[])
{
    // uint32_t time[100];
    // uint32_t p_avg = 0, c_avg = 0;
    // for (int i = 0; i < 100; i++) {
    for (size_t i = 0; i < 65536ULL; i++) {
        in[i] = i;
        out[i] = 0ULL;
    }

    rbuf_t r;
    r.num_threads = NUM_THREADS;
    r.messages_per_thread = 65536U;
    size_t buffer_size = BUFFER_SIZE;
    *r.q.consumer_ptr = out;

    /* In order to customize messages per thread and the buffer size, 
     * we use argv to recieve arguments. For specific number of messages, 
     * add 'm' at the front of the argument, for specific buffer size, add 
     * 'b' at the front of the argument.
     */
    if (argc > 2) {
        if (argv[1][0] == 'm') {
            r.messages_per_thread = (uint32_t) atoi(argv[1] + 1);
            buffer_size = (uint64_t) atoi(argv[2] + 1);
        }
        if (argv[1][0] == 'b') {
            buffer_size = (uint64_t) atoi(argv[1] + 1);
            r.messages_per_thread = (uint32_t) atoi(argv[2] + 1);
        }
    }

    if (argc > 1) {
        if (argv[1][0] == 'm')
            r.messages_per_thread = (uint32_t) atoi(argv[1] + 1);
        if (argv[1][0] == 'b')
            buffer_size = (uint64_t) atoi(argv[1] + 1);
    }

        // printf("r.message_per_thread is %u\n", r.messages_per_thread);
        // printf("buffer_size is %lu\n", buffer_size);
    
    queue_init(&r.q, buffer_size);

        // uint64_t start = get_time();

    pthread_t publisher_th;
    pthread_t consumer_th[NUM_THREADS];

    pthread_attr_t attr;
    pthread_attr_init(&attr);

        // printf("Starting publisher thread...\n");

    pthread_create(&publisher_th, &attr, &publisher_loop, (void *) &r);

        // printf("Starting consumer thread...\n");

    for(intptr_t i = 0; i < NUM_THREADS; i++)
        pthread_create(&consumer_th[i], &attr, &consumer_loop, (void *) &r);

    intptr_t sent;
    pthread_join(publisher_th, (void **) &sent);

    intptr_t recd[NUM_THREADS];
    for(intptr_t i = 0; i < NUM_THREADS; i++)
        pthread_join(consumer_th[i], (void **) &recd[i]);

        // uint64_t end = get_time();
        // time[i] = end - start;

        // p_avg += r.q.p_times;
        // c_avg += r.q.c_times;

        // for(size_t i = 0ULL; i < r.messages_per_thread; i++)
        //     printf("%lu\n", out[i]);

        // printf("\npublisher sent %ld messages\n", sent);
        // printf("consumer received %ld messages\n", recd);

    pthread_attr_destroy(&attr);

    queue_destroy(&r.q);
    // }
    
    // for(size_t i = 0; i < r.messages_per_thread; i++)
    //     printf("%ld\n", out[i]);

    for(size_t i = 0; i < 65536U; i++)
        printf("%ld\n", out[i]);

    // qsort(time, 100U, sizeof(uint32_t), comp);
    // long long avg = 0LL;
    // for (int num = 16; num < 84; num++) {
    //     avg += time[num];
    // }
    // avg /= 68;
    // // printf("average time : %lldus\n", avg);
    // // printf("With message size %llu, ", SIZE_OF_MESSAGE);
    // printf("average run time = %lldus\n", avg);
    // printf("%lld\n", avg);
    // printf("Average p : %u times\n", p_avg/100);
    // printf("Average c : %u times\n", c_avg/100);

    return 0;
}