// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include <sys/time.h>
#include <stdint.h>

typedef enum {
  READY,
  RUNNING,
  SUSPENDED,
  EXITED,
} task_status;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  
  ucontext_t context ;			// contexto armazenado da tarefa
  
  task_status status ;			// pronta, rodando, suspensa, ...
  int exit_code;
  
  int vg_id;

  int priority;
  int hqueue_index;
  int ready_queue_index;

  int clock_ticks;
  uint64_t last_schedule_tick;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
