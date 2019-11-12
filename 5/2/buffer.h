#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>

void init_locks(void);
void *producer(void *p_id);
void *consumer(void *p_id);

#endif // BUFFER_H
