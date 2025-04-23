#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ppos.h"
#include "queue.h"

#define PRODUCERS 10
#define CONSUMERS 2
#define BUFFER_LEN 4

task_t producers[PRODUCERS];
task_t consumers[CONSUMERS];

semaphore_t s_item, s_consumed, s_buffer;

typedef struct item_t {
    struct item_t *prev, *next;
    int value;
} item_t;


item_t items[BUFFER_LEN];
item_t *produced_items = NULL;
item_t *empty_items = NULL;

void producer(void *arg) {
    item_t *item = NULL;

    while (1) {
        task_sleep(5000);

        sem_down(&s_consumed);

        sem_down(&s_buffer);
            item = empty_items;
            assert(item != NULL);
            assert(queue_remove((queue_t**)&empty_items, (queue_t*)item) >= 0);
            
            item->value = random()%101;
            assert(queue_append((queue_t**)&produced_items, (queue_t*)item) >= 0);
            
            printf("Producer %ld produced %d\n", (long int)arg, item->value);
        sem_up(&s_buffer);
        
        sem_up(&s_item);
        item = NULL;
    }
}

void consumer(void *arg) {
    item_t *item = NULL;

    while (1) {
        sem_down(&s_item);

        sem_down(&s_buffer);
            item = produced_items;
            assert(item != NULL);
            assert(queue_remove((queue_t**)&produced_items, (queue_t*)item) >= 0);
            
            printf("\t\tConsumer %ld consumed %d\n", (long int)arg, item->value);
            assert(queue_append((queue_t**)&empty_items, (queue_t*)item) >= 0);
        sem_up(&s_buffer);
        
        sem_up(&s_consumed);
        item = NULL;

        task_sleep(1000);
    }
}

int main() {
    ppos_init();

    for (size_t i = 0; i < BUFFER_LEN; i++) {
        queue_append((queue_t**)&empty_items, (queue_t*)&items[i]);
    }
    
    sem_init(&s_consumed, BUFFER_LEN);
    sem_init(&s_buffer, BUFFER_LEN);
    sem_init(&s_item, 0);

    for (long int i = 0; i < PRODUCERS; i++) {
        task_init(&producers[i], &producer, (void *)i);
    }
    
    for (long int i = 0; i < CONSUMERS; i++) {
        task_init(&consumers[i], &consumer, (void *)i);
    }

    task_sleep(30000);
    return 0;
}