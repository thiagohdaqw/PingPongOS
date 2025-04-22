#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

extern task_t *current_task;
semaphore_t *semaphores = NULL;


int sem_init (semaphore_t *s, int value) {
    s->value = value;
    s->waiting_tasks = NULL;
    queue_append((queue_t**)&semaphores, (queue_t*)s);
    DEBUG_PRINT("[%d] sem_init: semaphore initilized", current_task->id);
    return 0;
}

int sem_down (semaphore_t *s) {
    assert(current_task != NULL);

    DEBUG_PRINT("[%d] sem_down: semaphore down with value %d\n", current_task->id, s->value);

    if (s->value > 0) {
        s->value--;
        return 0;
    } else {
        do {
            if (queue_append((queue_t**)&s->waiting_tasks, (queue_t*)current_task) < 0) {
                fprintf(stderr, "ERROR sem_down: failed to append current_task %d to waiting queue\n", current_task->id);
                return -1;
            }
            DEBUG_PRINT("[%d] sem_down: waiting semaphore\n", current_task->id);
            current_task->status = WAITING;
            task_yield();
            queue_remove((queue_t**)&s->waiting_tasks, (queue_t*)current_task);
            if (s->value == -1)
                return -1;
        } while (s->value <= 0);
        s->value--;
        return 0;
    }
}

void poll_waiting_tasks(semaphore_t *s) {
    task_t *current = s->waiting_tasks;
    int i = 0;

    while (current != NULL) {
        if (s->value != -1 && i >= s->value)
            break;

        task_t *next = current->next;

        DEBUG_PRINT("[%d] poll_waiting_tasks: awaking task %d with s->value %d\n", current_task->id, current->id, s->value);
        task_awake(current, &s->waiting_tasks);
        if (next == s->waiting_tasks || next == current)
            break;

        current = next;
        i++;
    }
}

int sem_up (semaphore_t *s) {
    s->value++;
    DEBUG_PRINT("[%d] sem_up: releasing a semaphore %d\n", current_task->id, s->value);
    poll_waiting_tasks(s);
    return 0;
}

int sem_destroy (semaphore_t *s) {
    DEBUG_PRINT("[%d] sem_destroy: destroying a semaphore\n", current_task->id);

    s->value = -1;

    poll_waiting_tasks(s);
    queue_remove((queue_t**)&semaphores, (queue_t*)s);
    return 0;
}