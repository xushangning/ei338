#include "buffer.h"

int main(int argc, char *argv[])
{
    struct producer_consumer_thread {
        pthread_t tid;
        int id;
    };
    int sleep_seconds, n_producers, n_consumers;
    sleep_seconds = atoi(argv[1]);
    n_producers = atoi(argv[2]);
    n_consumers = atoi(argv[3]);

    init_locks();
    struct producer_consumer_thread producers[n_producers];
    struct producer_consumer_thread consumers[n_consumers];
    for (int i = 0; i < n_producers; ++i) {
        producers[i].id = i;
        pthread_create(&producers[i].tid, NULL, producer, &producers[i].id);
    }
    for (int i = 0; i < n_consumers; ++i) {
        consumers[i].id = i;
        pthread_create(&consumers[i].tid, NULL, consumer, &consumers[i].id);
    }

    sleep(sleep_seconds);

    void *exit_status;
    for (int i = 0; i < n_producers; ++i) {
        pthread_cancel(producers[i].tid);
        pthread_join(producers[i].tid, &exit_status);
        if (exit_status == PTHREAD_CANCELED)
            printf("Producer %d has been canceled successfully.\n", producers[i].id);
    }
    for (int i = 0; i < n_consumers; ++i) {
        pthread_cancel(consumers[i].tid);
        pthread_join(consumers[i].tid, &exit_status);
        if (exit_status == PTHREAD_CANCELED)
            printf("Consumer %d has been canceled successfully.\n", consumers[i].id);
    }
    return 0;
}
