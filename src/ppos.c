#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <assert.h>
#include <valgrind/valgrind.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#include"ppos.h"
#include"pqueue.h"
#include"queue.h"

#define TASK_STATS

#define STACK_SIZE 64*1024
#define MAX_TASK_PRIORITY 20
#define TASK_TIMER_TICK 20
#define DISPATCHER_TIMER_INTERVAL_MS 1

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static int last_task_id = 0;

task_t *current_task = NULL;

static task_t *dispatcher_scheduled_task = NULL;
static task_t main_task = {0};
static task_t dispatcher_task = {0};

static pqueue_t ready_queue = {0};
static pqueue_t sleep_queue = {0};
static task_t *suspended_tasks = NULL; 

static unsigned int current_tick = 0;
static struct itimerval dispatcher_timer = {
    .it_interval = {
        .tv_usec = DISPATCHER_TIMER_INTERVAL_MS*1000,
    },
    .it_value = {
        .tv_usec = 1,
    }
};

void dispatcher(void *);
void dispatcher_timer_handler(int);

int min_task_priority_comparator(void *a, void *b) {
    int result = ((task_t*)a)->priority - ((task_t*)b)->priority;
    if (result == 0) {
        return ((task_t*)a)->last_schedule_tick - ((task_t*)b)->last_schedule_tick;
    }
    return result;
}

int min_task_sleep_comparator(void *a, void *b) {
    return ((task_t*)a)->sleep_expiration_tick - ((task_t*)b)->sleep_expiration_tick;
}

void update_task_ready_queue_index(void *a, int index) {
    ((task_t*)a)->ready_queue_index = index;
}

void ppos_init () {
    setvbuf (stdout, 0, _IONBF, 0) ;

    signal(SIGALRM, &dispatcher_timer_handler);
    if (setitimer(ITIMER_REAL, &dispatcher_timer, 0) < 0) {
        fprintf(stderr, "ERROR ppos_init: failed to init dispatcher timer\n");
        exit(1);
    }

    ready_queue = pqueue_init(min_task_priority_comparator, update_task_ready_queue_index);
    sleep_queue = pqueue_init(min_task_sleep_comparator, NULL);

    main_task.id = 0;
    main_task.status = RUNNING;
    main_task.clock_ticks = TASK_TIMER_TICK;
    current_task = &main_task;
    dispatcher_scheduled_task = current_task;
    current_task->last_schedule_tick = current_tick;

    task_init(&dispatcher_task, dispatcher, NULL);
}

void task_ready(task_t *task) {
    task->status = READY;
    pqueue_append(&ready_queue, (void*)task);
    DEBUG_PRINT("[%d] task_ready: turning task %d ready\n", current_task->id, task->id);
}

int task_init (task_t *task, void (*start_routine)(void *),  void *arg) {
    getcontext(&task->context);
    void *stack = malloc(STACK_SIZE);
    if (!stack) {
        fprintf(stderr, "ERROR: failed to alloc task stack");
        return -1;
    }

    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACK_SIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;

    task->id = ++last_task_id;
    task->vg_id = VALGRIND_STACK_REGISTER(stack, stack + STACK_SIZE);
    task->priority = 0;

    task->clock_ticks = DISPATCHER_TIMER_INTERVAL_MS;
    task->last_schedule_tick = current_tick;
    task->tick_start = current_tick;
    task->waiting_task_id = -1;
    makecontext(&task->context, (void*)(*start_routine), 1, arg);

    task_ready(task);

    #ifdef DEBUG
    printf ("task_init: iniciada tarefa %d\n", task->id) ;
    #endif
    return 0;
}

int task_switch (task_t *task) {
    #ifdef DEBUG
    printf ("task_switch: trocando contexto %d -> %d\n", current_task->id, task->id) ;
    #endif

    ucontext_t *current_context = &current_task->context;
    if (current_task->status == RUNNING)
        current_task->status = READY;

    current_task = task;
    current_task->status = RUNNING;
    return swapcontext(current_context, &task->context);
}

void print_task_exit_stats(task_t *task) {
    #ifndef TASK_STATS
    return;
    #endif
    unsigned int cpu_time = task->tick_used*DISPATCHER_TIMER_INTERVAL_MS;
    unsigned int execution_time = (current_tick-task->tick_start)*DISPATCHER_TIMER_INTERVAL_MS;
    printf(
        "Task %d exit: code %d, execution time %u ms, processor time %d ms, %d activations\n",
        task->id,
        task->exit_code,
        execution_time,
        cpu_time,
        task->activations
    );
}

void task_exit (int exit_code) {
    if (current_task == &main_task) {
        print_task_exit_stats(&main_task);
        int exit_code = task_wait(&dispatcher_task);
        print_task_exit_stats(&dispatcher_task);
        exit(exit_code);
    }

    DEBUG_PRINT("task_exit: tarefa %d sendo encerrada\n", current_task->id);

    current_task->status = EXITED;
    current_task->exit_code = exit_code;
    
    print_task_exit_stats(current_task);
    
    if (current_task != &dispatcher_task) {
        task_switch(&dispatcher_task);
    } else {
        task_switch(&main_task);
    }
}

void task_yield () {
    if (current_task->status == RUNNING) {
        task_ready(current_task);
    }
    DEBUG_PRINT("[%d] task_yield: yielding task with status %d\n", current_task->id, current_task->status);
    task_switch(&dispatcher_task);
}

int task_id () {
    return current_task->id;
}

void task_destroy(task_t *task) {
    if (task->vg_id) {
        VALGRIND_STACK_DEREGISTER(task->vg_id);
        task->vg_id = 0;
    }
    if (task->context.uc_stack.ss_sp != NULL) {
        free(task->context.uc_stack.ss_sp);
        task->context.uc_stack.ss_sp = NULL;         
    }
}

void task_setprio (task_t *task, int prio) {
    if (task == NULL)
        task = current_task;
    
    task->priority = MAX(MIN(prio, MAX_TASK_PRIORITY), -MAX_TASK_PRIORITY);

    if (task->ready_queue_index >= 0) {
        pqueue_update(&ready_queue, task->ready_queue_index);
    }
}

int task_getprio (task_t *task) {
    if (task == NULL)
        task = current_task;
    return task->priority;
}

void awake_suspended_tasks(task_t *exited_task);
void poll_sleeping_tasks();

void dispatcher(void *) {
    pqueue_remove(&ready_queue, dispatcher_task.ready_queue_index);

    do {
        while (pqueue_size(&ready_queue) > 0) {
            // TODO: improve cpu scheduling to minimize starvation
            dispatcher_scheduled_task = (task_t*)pqueue_pop(&ready_queue);
            
            dispatcher_scheduled_task->last_schedule_tick = current_tick;
            dispatcher_scheduled_task->clock_ticks = DISPATCHER_TIMER_INTERVAL_MS;
            dispatcher_scheduled_task->activations += 1;
            dispatcher_scheduled_task->status = RUNNING;
            task_setprio(dispatcher_scheduled_task, dispatcher_scheduled_task->priority+1);

            DEBUG_PRINT ("dispatcher: switching to task %d\n", dispatcher_scheduled_task->id) ;

            if (task_switch(dispatcher_scheduled_task) < 0) {
                fprintf(stderr, "ERROR dispatcher: failed to switch to task %d\n", dispatcher_scheduled_task->id);
                task_exit(-1);
            }

            DEBUG_PRINT ("dispatcher: task %d returned with status %d\n", dispatcher_scheduled_task->id, dispatcher_scheduled_task->status);

            switch (dispatcher_scheduled_task->status) {
                case EXITED:
                    awake_suspended_tasks(dispatcher_scheduled_task);
                    task_destroy(dispatcher_scheduled_task);
                    break;
                default:
                    break;
            }

            dispatcher_scheduled_task = NULL;

            poll_sleeping_tasks();
        }
        poll_sleeping_tasks();
        if (pqueue_size(&ready_queue) == 0) {
            pause();
            poll_sleeping_tasks();
        }
    } while (pqueue_size(&sleep_queue) > 0 || pqueue_size(&ready_queue) > 0);
    
    
    task_exit(0);
}

void dispatcher_timer_handler(int signum) {
    current_tick += 1;
    assert(current_tick < UINT32_MAX && "TODO current_tick overflow");

    current_task->tick_used += 1;

    if (current_task == NULL || dispatcher_scheduled_task == NULL || current_task == &dispatcher_task)
        return;
    dispatcher_scheduled_task->clock_ticks -= 1;
    if (dispatcher_scheduled_task->clock_ticks < 0) {
        DEBUG_PRINT("[%d] dispatcher_timer_handler: task has reached limit tick", current_task->id);
        task_yield();
    }
}

unsigned int systime() {
    return current_tick;
}

int task_wait (task_t *task) {
    if (task == NULL || task == current_task) {
        return -1;
    }
    if (task->status == EXITED) {
        return task->exit_code;
    }
    task->has_suspended_tasks = true;
    current_task->waiting_task_id = task->id;
    current_task->status = SUSPENDED;

    queue_append((queue_t**)&suspended_tasks, (queue_t*)current_task);
    
    DEBUG_PRINT("[%d] task_wait: waiting for task %d\n", current_task->id, task->id);

    task_yield();

    DEBUG_PRINT("[%d] task_wait: awaked from task %d\n", current_task->id, task->id);
    return task->exit_code;
}

void task_awake (task_t *task, task_t **queue) {
    if (queue_remove((queue_t**)queue, (queue_t*)task) < 0) {
        fprintf(stderr, "ERROR task_awake: failed to remove task %d from queue\n", task->id);
    }
    task->waiting_task_id = -1;

    DEBUG_PRINT("task_awake: task %d awaking task %d\n", current_task->id, task->id);
    task_ready(task);
}

void awake_suspended_tasks(task_t *exited_task) {
    if (!exited_task->has_suspended_tasks)
        return;
    task_t *suspended_task = suspended_tasks;
    while (suspended_task != NULL) {
        task_t *next_task = suspended_task->next;

        if (suspended_task->waiting_task_id == exited_task->id) {
            task_awake(suspended_task, &suspended_tasks);
        } else if (next_task == suspended_tasks || next_task == current_task)
            break;
        suspended_task = next_task;
    }
}

void task_sleep (int t) {
    unsigned int expiration = current_tick + (t/DISPATCHER_TIMER_INTERVAL_MS);
    current_task->sleep_expiration_tick = expiration;
    pqueue_append(&sleep_queue, (void*)current_task);
    current_task->status = SLEEPING;

    DEBUG_PRINT("task_sleep: task %d sleeping for %d ms\n", current_task->id, t);
    task_yield();
    DEBUG_PRINT("task_sleep: task %d awaked from sleeping\n", current_task->id, t);
}

void poll_sleeping_tasks() {
    while (pqueue_size(&sleep_queue) > 0
        && ((task_t*)pqueue_peek(&sleep_queue))->sleep_expiration_tick <= current_tick) {
        task_t *task = pqueue_pop(&sleep_queue);
        task_ready(task);
    }
}