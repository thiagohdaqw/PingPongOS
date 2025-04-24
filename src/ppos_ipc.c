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

// TODO: add check to verify sem is not destroyied
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
        if (next == current)
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

int sem_up_n(semaphore_t *s, int inc) {
    s->value += inc;
    DEBUG_PRINT("[%d] sem_up_n: releasing a semaphore %d\n", current_task->id, s->value);
    poll_waiting_tasks(s);
    return 0;
}

int sem_destroy (semaphore_t *s) {
    DEBUG_PRINT("[%d] sem_destroy: destroying a semaphore\n", current_task->id);

    s->value = -1;

    poll_waiting_tasks(s);
    return queue_remove((queue_t**)&semaphores, (queue_t*)s);
}

int mutex_init (mutex_t *m) {
    DEBUG_PRINT("[%d] mutex_init: mutex initialization\n", current_task->id);
    return sem_init(&m->sem, 1);
}

// TODO: add verification to check if the mutex is destroyed
int mutex_lock (mutex_t *m) {
    DEBUG_PRINT("[%d] mutex_lock: mutex locking\n", current_task->id);
    return sem_down(&m->sem);
}

int mutex_unlock (mutex_t *m) {
    DEBUG_PRINT("[%d] mutex_unlock: mutex unlock\n", current_task->id);
    return sem_up(&m->sem);
}

int mutex_destroy (mutex_t *m) {
    DEBUG_PRINT("[%d] mutex_destroy: mutex destroy\n", current_task->id);
    return sem_destroy(&m->sem);
}

int barrier_init (barrier_t *b, int n) {
    DEBUG_PRINT("[%d] barrier_init: barrier initialization with N = %d\n", current_task->id, n);

    b->count = n;
    b->value = 0;
    return sem_init(&b->sem, 0);
}

// TODO: add verification to check if the barrier is destroyed
int barrier_join (barrier_t *b) {
    if (b->value + 1 < b->count) {
        b->value++;
        DEBUG_PRINT("[%d] barrier_join: waiting in barrier %d/%d\n", current_task->id, b->value, b->count);
        return sem_down(&b->sem);
    }
    DEBUG_PRINT("[%d] barrier_join: barrier releasing %d/%d\n", current_task->id, b->value+1, b->count);
    b->value = 0;
    return sem_up_n(&b->sem, b->count-1);
}

int barrier_destroy (barrier_t *b) {
    DEBUG_PRINT("[%d] barrier_destroy: barrier destroying\n", current_task->id, b->value+1, b->count);
    return sem_destroy(&b->sem);
}
