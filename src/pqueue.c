#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "pqueue.h"

pqueue_t pqueue_init(int (*comparator)(void *, void *), void (*update_index)(void *, int index))
{
    pqueue_t queue = {0};
    queue.capacity = 0;
    queue.count = 0;
    queue.items = NULL;
    queue.comparator = comparator;
    queue.update_index = update_index;
    return queue;
}

int pqueue_size(pqueue_t *queue)
{
    return queue->count;
}

void shift_up(pqueue_t *queue, int index);
void shift_down(pqueue_t *queue, int index);

void set_item(pqueue_t *queue, void *item, int index)
{
    queue->items[index] = item;
    if (queue->update_index != NULL)
        queue->update_index(item, index);
}

void swap_item(pqueue_t *queue, int a, int b)
{
    void *temp = queue->items[a];
    set_item(queue, queue->items[b], a);
    set_item(queue, temp, b);
}

int left_index(int index)
{
    return 2 * index + 1;
}

int right_index(int index)
{
    return 2 * index + 2;
}

int parent_index(int index)
{
    return (index - 1) / 2;
}

int pqueue_append(pqueue_t *queue, void *item)
{
    if (queue->count + 1 >= queue->capacity)
    {
        if (queue->capacity == 0)
            queue->capacity = 256;
        else
            queue->capacity *= 2;
        queue->items = queue->items == NULL
                           ? calloc(queue->capacity, sizeof(*queue->items))
                           : realloc(queue->items, queue->capacity * sizeof(*queue->items));
        if (queue->items == NULL)
        {
            fprintf(stderr, "ERROR pqueue_append: failed to alloc queue->items\n");
            return 1;
        }
    }
    set_item(queue, item, queue->count++);
    shift_up(queue, queue->count - 1);
    return 0;
}
void *pqueue_pop(pqueue_t *queue)
{
    assert(queue->count > 0 && "Queue empty");
    return pqueue_remove(queue, 0);
}

void pqueue_update(pqueue_t *queue, int index)
{
    assert(index >= 0);
    shift_down(queue, index);
    shift_up(queue, index);
}

void* pqueue_remove(pqueue_t *queue, int index) {
    void *item = queue->items[index];
    swap_item(queue, index, --queue->count);
    shift_down(queue, index);
    shift_up(queue, index);

    if (queue->update_index != NULL) {
        queue->update_index(item, -1);
    }

    return item;
}

void shift_up(pqueue_t *queue, int index)
{
    while (index > 0)
    {
        int parent = parent_index(index);
        if (queue->comparator(queue->items[index], queue->items[parent]) < 0)
        {
            swap_item(queue, index, parent);
            index = parent;
        }
        else
            break;
    }
}

void shift_down(pqueue_t *queue, int index)
{
    while ((2 * index) + 1 < queue->count)
    {
        int max = index;
        int left = left_index(index);
        if (left < queue->count && queue->comparator(queue->items[max], queue->items[left]) > 0)
        {
            max = left;
        }
        int right = right_index(index);
        if (right < queue->count && queue->comparator(queue->items[max], queue->items[right]) > 0)
        {
            max = right;
        }

        if (max == index)
            break;
        swap_item(queue, index, max);
        index = max;
    }
}
