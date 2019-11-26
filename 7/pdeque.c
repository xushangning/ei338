#include "pdeque.h"

#include <limits.h>
#include <string.h>

struct pdeque *pdeque_new(int size)
{
    struct pdeque *d = (struct pdeque *) malloc(sizeof(struct pdeque));
    d->head = NULL;
    d->size = size;
    return d;
}

void pdeque_node_free(struct pdeque_node *n)
{
    free(n->name);
    free(n);
}

void pdeque_free(struct pdeque *d)
{
    struct pdeque_node *n = d->head, *next;
    while (n) {
        next = n->next;
        pdeque_node_free(n);
        n = next;
    }
    free(d);
}

void pdeque_insert_after(struct pdeque_node *p, struct pdeque_node *new)
{
    new->next = p->next;
    p->next = new;
    new->prev = p;
    if (new->next)
        new->next->prev = new;
}

void pdeque_prepend(struct pdeque *d, struct pdeque_node *new)
{
    new->next = d->head;
    d->head = new;
    new->prev = NULL;
    if (new->next)
        new->next->prev = new;
}

bool request(struct pdeque *d, const char *name, int size, enum strategy st)
{
    bool successful = false;

    /**
     * If selected == NULL, the hole at the start will be allocated.
     * Otherwise, the hole after selected will be allocated.
     */
    struct pdeque_node *selected = NULL;

    if (d->head) {
        if (d->head->start > size)
            successful = true;

        int optimal_hole_size, temp_hole_size;
        switch (st) {
            case FIRST_FIT:
                if (d->head->start < size) {
                    for (struct pdeque_node *p = d->head; p; p = p->next) {
                        // If p->next == NULL, then p is the process with the
                        // largest address. The hole after p should be
                        // calculated (d->size - p->end).
                        if ((p->next && p->next->start - p->end > size)
                                || (!p->next && d->size - p->end > size)) {
                            selected = p;
                            successful = true;
                            break;
                        }
                    }
                }
                break;
            case BEST_FIT:
                optimal_hole_size = successful ? d->head->start : INT_MAX;
                for (struct pdeque_node *p = d->head; p; p = p->next) {
                    temp_hole_size = (p->next ? p->next->start : d->size) - p->end;
                    if (temp_hole_size > size && temp_hole_size < optimal_hole_size) {
                        optimal_hole_size = temp_hole_size;
                        selected = p;
                        successful = true;
                    }
                }
                break;
            case WORST_FIT:
                optimal_hole_size = d->head->start;
                for (struct pdeque_node *p = d->head; p; p = p->next) {
                    temp_hole_size = (p->next ? p->next->start : d->size) - p->end;
                    if (temp_hole_size > size && temp_hole_size > optimal_hole_size) {
                        optimal_hole_size = temp_hole_size;
                        selected = p;
                        successful = true;
                    }
                }
                break;
        }
    }
    else if (d->size > size)
        successful = true;

    if (successful) {
        struct pdeque_node *new = (struct pdeque_node *) malloc(sizeof(struct pdeque_node));
        size_t name_size = strlen(name) + 1;
        new->name = (char *) calloc(name_size, sizeof(char));
        strncpy(new->name, name, name_size);

        if (selected) {
            new->start = selected->end;
            new->end = selected->end + size;
            pdeque_insert_after(selected, new);
        }
        else {
            new->start = 0;
            new->end = size;
            pdeque_prepend(d, new);
        }
    }
    return successful;
}

void release(struct pdeque *d, const char *name)
{
    for (struct pdeque_node *p = d->head; p; p = p->next) {
        if (!strncmp(p->name, name, PROCESS_NAME_MAX_LENGTH)) {
            if (p->prev)
                p->prev->next = p->next;
            else
                d->head = p->next;

            if (p->next)
                p->next->prev = p->prev;

            pdeque_node_free(p);
            break;
        }
    }
}

void compact(struct pdeque *d)
{
    int prev_end_addr = 0;
    for (struct pdeque_node *p = d->head; p; p = p->next) {
        p->end -= p->start - prev_end_addr;
        p->start = prev_end_addr;
        prev_end_addr = p->end;
    }
}

void stat(struct pdeque *d)
{
    if (d->head) {
        if (d->head->start)
            printf("Addresses [0:%d] Unused\n", d->head->start);

        struct pdeque_node *p;
        for (p = d->head; p->next; p = p->next) {
            printf("Addresses [%d:%d] %s\n", p->start, p->end, p->name);
            if (p->next->start != p->end)
                printf("Addresses [%d:%d] Unused\n", p->end, p->next->start);
        }

        // the last process
        printf("Addresses [%d:%d] %s\n", p->start, p->end, p->name);
        // the unused memory after the last process
        if (d->size != p->end)
            printf("Addresses [%d:%d] Unused\n", p->end, d->size);
    }
    else
        printf("Addresses [0:%d] Unused\n", d->size);
}
