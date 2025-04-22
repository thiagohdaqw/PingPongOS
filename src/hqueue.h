#ifndef __INCLUDE_HQUEUE_H
#define __INCLUDE_HQUEUE_H


typedef struct
{
    void **items;
    int count;
    int capacity;
    int (*comparator)(void*, void*);
    void (*update_index)(void*, int index);
} hqueue_t;

hqueue_t hqueue_init(int (*comparator)(void*, void*), void (*update_index)(void*, int));
int hqueue_size(hqueue_t *queue);

int hqueue_append(hqueue_t *queue, void *item);
void* hqueue_pop(hqueue_t *queue);
void hqueue_update(hqueue_t *queue, int index);
void* hqueue_remove(hqueue_t *queue, int index);
#endif