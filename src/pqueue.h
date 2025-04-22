#ifndef __INCLUDE_PQUEUE_H
#define __INCLUDE_PQUEUE_H


typedef struct
{
    void **items;
    int count;
    int capacity;
    int (*comparator)(void*, void*);
    void (*update_index)(void*, int index);
} pqueue_t;

pqueue_t pqueue_init(int (*comparator)(void*, void*), void (*update_index)(void*, int));
int pqueue_size(pqueue_t *queue);

int pqueue_append(pqueue_t *queue, void *item);
void* pqueue_pop(pqueue_t *queue);
void* pqueue_peek(pqueue_t *queue);
void pqueue_update(pqueue_t *queue, int index);
void* pqueue_remove(pqueue_t *queue, int index);
#endif