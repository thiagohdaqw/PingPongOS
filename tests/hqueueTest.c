#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

#include "hqueue.h"
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
    ((task_t*)a)->hqueue_index = index;
}

int main() {
    hqueue_t queue = hqueue_init(&max_comparator, &update_index);

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = i;
        hqueue_append(&queue, (void*)&items[i]);
        assert(hqueue_size(&queue) == i+1);
    }

    printf("MAX: ");
    for (size_t i = 0; i < MAX ; i++) {
        task_t *item = (task_t*)hqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(hqueue_size(&queue) == MAX-i-1);
        assert(item->priority == MAX-i-1);
    }
    printf("\n");
    assert(hqueue_size(&queue) == 0);

    queue.comparator = &min_comparator;

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = i;
        hqueue_append(&queue, (void*)&items[i]);
        assert(hqueue_size(&queue) == i+1);
    }

    printf("MIN: ");
    for (size_t i = 0; i < MAX ; i++) {
        task_t *item = (task_t*)hqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(hqueue_size(&queue) == MAX-i-1);
        assert(item->priority == i);
    }
    printf("\n");
    assert(hqueue_size(&queue) == 0);

    srandom(time(NULL));

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = (int)(((float)rand()/RAND_MAX)*MAX);
        hqueue_append(&queue, (void*)&items[i]);
        assert(hqueue_size(&queue) == i+1);
    }

    printf("RAND MIN: ");
    int last = INT_MIN;
    while (hqueue_size(&queue) > 0) {
        task_t *item = (task_t*)hqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(item->priority >= last);
        last = item->priority;
    }
    printf("\n");

    queue.comparator = &max_comparator;

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = (int)(((float)rand()/RAND_MAX)*MAX);
        hqueue_append(&queue, (void*)&items[i]);
        assert(hqueue_size(&queue) == i+1);
    }

    printf("RAND MAX: ");
    last = INT_MAX;
    while (hqueue_size(&queue) > 0) {
        task_t *item = (task_t*)hqueue_pop(&queue);
        printf("%d ", item->priority);
        assert(item->priority <= last);
        last = item->priority;
    }
    printf("\n");


    queue.comparator = &min_comparator;

    for (size_t i = 0; i < MAX; i++) {
        items[i].priority = MAX - 1;
        hqueue_append(&queue, (void*)&items[i]);
        assert(hqueue_size(&queue) == i+1);
    }


    items[0].priority = -1234;
    hqueue_update(&queue, items[0].hqueue_index);

    items[3].priority = -123;
    hqueue_update(&queue, items[3].hqueue_index);

    assert(((task_t*)hqueue_pop(&queue))->priority == -1234);
    assert(((task_t*)hqueue_pop(&queue))->priority == -123);
}