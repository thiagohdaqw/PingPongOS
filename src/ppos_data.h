// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <ucontext.h>  // biblioteca POSIX de trocas de contexto

typedef enum {
    READY,
    RUNNING,
    SUSPENDED,
    SLEEPING,
    WAITING,
    EXITED,
} task_status;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t {
    struct task_t *prev, *next;  // ponteiros para usar em filas
    int id;                      // identificador da tarefa

    ucontext_t context;  // contexto armazenado da tarefa

    task_status status;  // pronta, rodando, suspensa, ...
    int exit_code;

    int vg_id;

    int priority;
    int pqueue_index;
    int ready_queue_index;

    int clock_ticks;
    unsigned int tick_used;
    unsigned int tick_start;
    unsigned int activations;
    unsigned int last_schedule_tick;

    int waiting_task_id;
    bool has_suspended_tasks;

    unsigned int sleep_expiration_tick;
} task_t;

// estrutura que define um semáforo
typedef struct semaphore_t {
    struct semaphore_t *prev, *next;
    int value;
    task_t *waiting_tasks;
} semaphore_t;

// estrutura que define um mutex
typedef struct {
    semaphore_t *sem;
    // preencher quando necessário
} mutex_t;

// estrutura que define uma barreira
typedef struct {
    // preencher quando necessário
} barrier_t;

// estrutura que define uma fila de mensagens
typedef struct {
    // preencher quando necessário
} mqueue_t;

#endif
