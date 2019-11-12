#include <semaphore.h>

#include "buffer.h"

typedef int buffer_item;

#define BUFFER_SIZE 6
static buffer_item buffer[BUFFER_SIZE];

static pthread_mutex_t mutex;
static sem_t full, empty;

/**
 * Points to the next free and first full position in the buffer,
 * respectively.
 *
 * Access to these variables is synchronized by the mutex lock.
 */
static int in, out;

void init_locks(void)
{
    pthread_mutex_init(&mutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
}

static int insert_item(buffer_item item)
{
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE;
    return 0;
}

static int remove_item(buffer_item *item) {
    *item = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    return 0;
}

/**
 * This doc applies both to the producer and consumer.
 *
 * The two functions will only exit at thread cancellation and will never
 * reach the return statement.
 *
 * The producer and consumer have two cancellation points: sleep() and
 * sem_wait(). fprintf() may be a cancellation point. We assume that
 * fprintf() is not a cancellation point so that production and consumption
 * can't be cancelled and thus we don't need to release the locks after
 * cancellation.
 *
 * @param p_id ID of the producer/consumer
 * @return NULL
 */
void *producer(void *p_id)
{
    int id = *(int *)p_id;
    buffer_item item;
    while (true) {
        sleep(rand() % 4);
        item = rand() % 1000;
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        if (insert_item(item))
            fprintf(stderr, "report error condition");
        else
            printf("Producer %d produced %d.\n", id, item);

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
    return NULL;
}

/**
 * See producer above for documentation.
 *
 * @param p_id
 * @return NULL
 */
void *consumer(void *p_id)
{
    int id = *(int *)p_id;
    buffer_item item;
    while (true) {
        sleep(rand() % 4);
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        if (remove_item(&item))
            fprintf(stderr, "report error condition");
        else
            printf("Consumer %d consumed %d.\n", id, item);

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
    return NULL;
}
