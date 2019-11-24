/**
 * A deque for processes
 */

#ifndef DEQUE_H
#define DEQUE_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define PROCESS_NAME_MAX_LENGTH 3

enum strategy {FIRST_FIT, BEST_FIT, WORST_FIT};

struct pdeque {
    struct pdeque_node *head;
    int size;           // total memory
};

struct pdeque_node {
    struct pdeque_node *prev, *next;
    int start, end;    // start and end address of the allocated memory
    char *name;         // process name
};

struct pdeque *pdeque_new(int size);
void pdeque_node_free(struct pdeque_node *n);
void pdeque_free(struct pdeque *d);
void pdeque_insert_after(struct pdeque_node *p, struct pdeque_node *new);
void pdeque_prepend(struct pdeque *d, struct pdeque_node *new);

bool request(struct pdeque *d, const char *name, int size, enum strategy st);
void release(struct pdeque *d, const char *name);
void compact(struct pdeque *d);
void stat(struct pdeque *d);

#endif // DEQUE_H
