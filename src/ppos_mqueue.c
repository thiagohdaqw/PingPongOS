#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ppos.h"
#include "queue.h"

#define MQUEUE_BUFFER_DATA(b) (b+sizeof(queue_t))
#define MQUEUE_BUFFER_AT(q, i) ((q)->buffer + ((i)*(q)->buffer_item_size))

extern task_t *current_task;


int mqueue_init (mqueue_t *queue, int max, int size) {
    DEBUG_PRINT("[%d] mqueue_init: mqueue initialization with max=%d and size=%d\n", current_task->id, max, size);

    queue->buffer_item_size = sizeof(queue_t)+size;
    queue->size = size;
    queue->count = 0;
    queue->capacity = max;
    queue->buffer = malloc(max*queue->buffer_item_size);
    assert(queue->buffer != NULL && "BUY MORE RAM LOL");

    if (mutex_init(&queue->mutex) < 0) return -1;
    if (sem_init(&queue->spares, max) < 0) return -1;
    if (sem_init(&queue->produceds, 0) < 0) return -1;

    return 0;
}

int mqueue_send (mqueue_t *queue, void *msg) {
    DEBUG_PRINT("[%d] mqueue_send: sending message", current_task->id);

    if (sem_down(&queue->spares)) return -1;

    if (mutex_lock(&queue->mutex) < 0) return -1;
    {
        void *buffer = MQUEUE_BUFFER_AT(queue, queue->buffer_index);
        void *buffer_data = MQUEUE_BUFFER_DATA(buffer);
        memset(buffer, 0, queue->buffer_item_size);

        memcpy(buffer_data, msg, queue->size);

        queue->buffer_index = (queue->buffer_index + 1) % queue->capacity;

        if (queue_append(&queue->items, (queue_t*)buffer) < 0) {
            mutex_unlock(&queue->mutex);
            return -1;
        }

        queue->count++;
    }
    if (mutex_unlock(&queue->mutex) < 0) return -1;

    if (sem_up(&queue->produceds) < 0) return -1;
    return 0;
}

int mqueue_recv (mqueue_t *queue, void *msg) {
    DEBUG_PRINT("[%d] mqueue_recv: receiving message", current_task->id);

    if (sem_down(&queue->produceds)) return -1;

    if (mutex_lock(&queue->mutex) < 0) return -1;
    {
        void *item = queue->items;
        assert(item != NULL);
        void *data = MQUEUE_BUFFER_DATA(item);

        memcpy(msg, data, queue->size);

        if (queue_remove((queue_t**)&queue->items, (queue_t*)item) < 0) {
            mutex_unlock(&queue->mutex);
            return -1;
        }

        queue->count--;
    }
    if (mutex_unlock(&queue->mutex) < 0) return -1;

    if (sem_up(&queue->spares) < 0) return -1;

    return 0;
}

int mqueue_destroy (mqueue_t *queue) {
    DEBUG_PRINT("[%d] mqueue_destroy: destroying", current_task->id);

    free(queue->buffer);
    sem_destroy(&queue->spares);
    sem_destroy(&queue->produceds);
    mutex_destroy(&queue->mutex);
    return 0;
}

int mqueue_msgs (mqueue_t *queue) {
    return queue->count;
}
