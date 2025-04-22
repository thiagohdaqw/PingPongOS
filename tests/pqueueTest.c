#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

#include "pqueue.h"
#include "ppos_data.h"

#define MAX 10
task_t items[MAX] = {0};


int max_comparator(void *a, void *b) {
    return ((task_t*)b)->priority - ((task_t*)a)->priority;
}

int min_comparator(void *a, void *b) {
    return ((task_t*)a)->priority - ((task_t*)b)->priority;
}

void update_index(void *a, int index) {
    ((task_t*)a)->pqueue_index = index;
}

int main() {
    pqueue_t queue = pqueue_init(&max_comparator, &update_index);

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = i;
        pqueue_append(&queue, (void*)&items[i]);
        assert(pqueue_size(&queue) == i+1);
    }

    printf("MAX: ");
    for (size_t i = 0; i < MAX ; i++) {
        task_t *item = (task_t*)pqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(pqueue_size(&queue) == MAX-i-1);
        assert(item->priority == MAX-i-1);
    }
    printf("\n");
    assert(pqueue_size(&queue) == 0);

    queue.comparator = &min_comparator;

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = i;
        pqueue_append(&queue, (void*)&items[i]);
        assert(pqueue_size(&queue) == i+1);
    }

    printf("MIN: ");
    for (size_t i = 0; i < MAX ; i++) {
        task_t *item = (task_t*)pqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(pqueue_size(&queue) == MAX-i-1);
        assert(item->priority == i);
    }
    printf("\n");
    assert(pqueue_size(&queue) == 0);

    srandom(time(NULL));

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = (int)(((float)rand()/RAND_MAX)*MAX);
        pqueue_append(&queue, (void*)&items[i]);
        assert(pqueue_size(&queue) == i+1);
    }

    printf("RAND MIN: ");
    int last = INT_MIN;
    while (pqueue_size(&queue) > 0) {
        task_t *item = (task_t*)pqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(item->priority >= last);
        last = item->priority;
    }
    printf("\n");

    queue.comparator = &max_comparator;

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = (int)(((float)rand()/RAND_MAX)*MAX);
        pqueue_append(&queue, (void*)&items[i]);
        assert(pqueue_size(&queue) == i+1);
    }

    printf("RAND MAX: ");
    last = INT_MAX;
    while (pqueue_size(&queue) > 0) {
        task_t *item = (task_t*)pqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(item->priority <= last);
        last = item->priority;
    }
    printf("\n");


    queue.comparator = &min_comparator;

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = MAX - 1;
        pqueue_append(&queue, (void*)&items[i]);
        assert(pqueue_size(&queue) == i+1);
    }

    items[0].priority = -1234;
    pqueue_update(&queue, items[0].pqueue_index);

    items[3].priority = -123;
    pqueue_update(&queue, items[3].pqueue_index);

    assert(((task_t*)pqueue_pop(&queue))->priority == -1234);
    assert(((task_t*)pqueue_pop(&queue))->priority == -123);
}