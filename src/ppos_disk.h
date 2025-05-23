// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include <stdbool.h>

#include "ppos.h"
// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

typedef enum disk_request_type {
    READ,
    WRITE,
} disk_request_type;

typedef struct disk_request_t {
    struct disk_request_t *prev, *next;
    disk_request_type type;
    task_t *requester;
    int block;
    void *buffer;
    int code;
} disk_request_t;


// estrutura que representa um disco no sistema operacional
typedef struct
{
    bool initializated;

    disk_request_t *requests;
    semaphore_t requests_sem;
    mutex_t mutex;
    mutex_t disk_result;
} disk_t ;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif