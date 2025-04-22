#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <assert.h>
#include <valgrind/valgrind.h>
#include <signal.h>
#include <sys/time.h>
#include <stdint.h>

#include"ppos.h"
#include"hqueue.h"

// #define DEBUG
#define STACK_SIZE 64*1024
#define MAX_TASK_PRIORITY 20
#define TASK_TIMER_TICK 20
#define DISPATCHER_TIMER_INTERVAL_MS 1

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


static int last_task_id = 0;

static task_t *current_task = NULL;
static task_t *dispatcher_scheduled_task = NULL;
static task_t main_task = {0};
static task_t dispatcher_task = {0};

static hqueue_t ready_queue = {0};

static uint64_t current_tick = 0;
static struct itimerval dispatcher_timer = {
    .it_interval = {
        .tv_usec = DISPATCHER_TIMER_INTERVAL_MS*1000,
    },
    .it_value = {
        .tv_usec = DISPATCHER_TIMER_INTERVAL_MS*1000,
    }
};

void dispatcher(void *);
void dispatcher_timer_handler(int);

int min_task_comparator(void *a, void *b) {
    int result = ((task_t*)a)->priority - ((task_t*)b)->priority;
    if (result == 0) {
        return ((task_t*)a)->last_schedule_tick - ((task_t*)b)->last_schedule_tick;
    }
    return result;
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

    ready_queue = hqueue_init(min_task_comparator, update_task_ready_queue_index);

    main_task.id = 0;
    main_task.status = RUNNING;
    current_task = &main_task;

    task_init(&dispatcher_task, dispatcher, NULL);
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
    task->status = READY;
    task->vg_id = VALGRIND_STACK_REGISTER(stack, stack + STACK_SIZE);
    task->priority = 0;

    task->clock_ticks = DISPATCHER_TIMER_INTERVAL_MS;
    task->last_schedule_tick = current_tick;
    makecontext(&task->context, (void*)(*start_routine), 1, arg);

    hqueue_append(&ready_queue, (void*)task);

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

void task_exit (int exit_code) {
    if (current_task == &main_task) {
        task_switch(&dispatcher_task);
        exit(dispatcher_task.exit_code);
    }

    #ifdef DEBUG
    printf ("task_exit: tarefa %d sendo encerrada\n", current_task->id) ;
    #endif

    current_task->status = EXITED;
    current_task->exit_code = exit_code;
    
    if (current_task != &dispatcher_task) {
        task_switch(&dispatcher_task);
    } else {
        task_switch(&main_task);
    }
}

void task_yield () {
    current_task->status = READY;
    hqueue_append(&ready_queue, (void*)current_task);
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
    assert(task != &main_task);
    
    task->priority = MAX(MIN(prio, MAX_TASK_PRIORITY), -MAX_TASK_PRIORITY);

    if (task->ready_queue_index >= 0) {
        hqueue_update(&ready_queue, task->ready_queue_index);
    }
}

int task_getprio (task_t *task) {
    if (task == NULL)
        task = current_task;
    return task->priority;
}

void dispatcher(void *) {
    hqueue_remove(&ready_queue, dispatcher_task.ready_queue_index);

    while (hqueue_size(&ready_queue) > 0) {
        dispatcher_scheduled_task = (task_t*)hqueue_pop(&ready_queue);
        
        dispatcher_scheduled_task->last_schedule_tick = current_tick;
        dispatcher_scheduled_task->clock_ticks = DISPATCHER_TIMER_INTERVAL_MS;
        task_setprio(dispatcher_scheduled_task, dispatcher_scheduled_task->priority+1);

        #ifdef DEBUG
            printf ("dispatcher: switching to task %d\n", dispatcher_scheduled_task->id) ;
        #endif

        if (task_switch(dispatcher_scheduled_task) < 0) {
            fprintf(stderr, "ERROR dispatcher: failed to switch to task %d\n", dispatcher_scheduled_task->id);
            task_exit(-1);
        }

        #ifdef DEBUG
            printf ("dispatcher: task %d returned with status %d\n", dispatcher_scheduled_task->id, dispatcher_scheduled_task->status);
        #endif

        switch (dispatcher_scheduled_task->status) {
            case EXITED:
                task_destroy(dispatcher_scheduled_task);
                break;
            default:
                break;
        }

        dispatcher_scheduled_task = NULL;
    }
    
    task_exit(0);
}

void dispatcher_timer_handler(int signum) {
    current_tick += 1;
    assert(current_tick < UINT64_MAX && "TODO current_tick overflow");
    if (dispatcher_scheduled_task == NULL || dispatcher_scheduled_task == &main_task || dispatcher_scheduled_task == &dispatcher_task)
        return;
    dispatcher_scheduled_task->clock_ticks -= 1;
    if (dispatcher_scheduled_task->clock_ticks < 0) {
        task_yield();
    }
}