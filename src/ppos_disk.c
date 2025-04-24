#include <assert.h>
#include <stdlib.h>
#include <signal.h>

#include "ppos_disk.h"
#include "disk.h"

extern task_t *current_task;

static disk_t state = {0};
static task_t driver_task = {0};
static disk_request_t *current_request = NULL;
static bool current_request_ready = false;

void disk_driver_body (void *);
void disk_handler(int);

int disk_mgr_init (int *numBlocks, int *blockSize) {
    if (state.initializated) {
        DEBUG_PRINT("[%d] disk_mgr_init: trying to initilizate a already initializated disk", current_task->id);
        return -1;
    }

    state.initializated = true;
    if (sem_init(&state.requests_sem, 0) < 0) goto error;
    if (mutex_init(&state.mutex) < 0) goto error;
    if (mutex_init(&state.disk_result) < 0) goto error;
    if (mutex_lock(&state.disk_result) < 0) goto error;
    if (task_init(&driver_task, &disk_driver_body, NULL) < 0) goto error;

    if (disk_cmd(DISK_CMD_INIT, 0, NULL) < 0) goto error;

    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, NULL);
    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, NULL);

    signal(SIGUSR1, &disk_handler);

    return 0;
    error:
        DEBUG_PRINT("[%d] disk_mgr_init: failed to initializate a disk", current_task->id);
        sem_destroy(&state.requests_sem);
        mutex_destroy(&state.mutex);
        return -1;
}

int disk_make_request(disk_request_type type, int block, void *buffer) {
    if (mutex_lock(&state.mutex) < 0) {
        DEBUG_PRINT("[%d] disk_add_request: failed to lock mutex\n", current_task->id);
        return -1;
    }
    int result = 0;
    disk_request_t *request = calloc(1, sizeof(disk_request_t));
    do {
        if (request == NULL) {
            result = -1;
            break;
        }

        request->type = type;
        request->requester = current_task;
        request->block = block;
        request->buffer = buffer;
        request->code = -1;

        if (queue_append((queue_t**)&state.requests, (queue_t*)request) < 0) {
            result = -1;
            break;
        }
    } while (0);

    if (mutex_unlock(&state.mutex) < 0) {
        result = -1;
        goto end;
    }

    if (result < 0) {
        goto end;
    }

    if (sem_up(&state.requests_sem) < 0) {
        result = -1;
        goto end;
    }

    task_suspend(NULL);
    result = request->code;
    end:
        free(request);
        return result;
}

int disk_block_read (int block, void *buffer) {
    return disk_make_request(READ, block, buffer);
}

int disk_block_write (int block, void *buffer) {
    return disk_make_request(WRITE, block, buffer);
}

void disk_driver_body (void *) {
    while (1) {
        current_request = NULL;
        current_request_ready = false;

        if (sem_down(&state.requests_sem) < 0) return;
        assert(state.requests != NULL);

        if (mutex_lock(&state.mutex) < 0) {
            DEBUG_PRINT("[%d] diskDriverBody: failed to lock mutex\n", current_task->id);
            continue;
        }
        disk_request_t *request = state.requests;
        queue_remove((queue_t**)&state.requests, (queue_t*)request);

        if (mutex_unlock(&state.mutex) < 0) {
            DEBUG_PRINT("[%d] diskDriverBody: failed to unlock mutex\n", current_task->id);
            return;
        }

        switch (request->type) {
        case READ:
            if (disk_cmd(DISK_CMD_READ, request->block, request->buffer) < 0) goto error;
            break;
        case WRITE:
            if (disk_cmd(DISK_CMD_WRITE, request->block, request->buffer) < 0) goto error;
            break;
        default:
            assert(0 && "UNREACHABLE");
        }

        mutex_lock(&state.disk_result);

        request->code = 0;
        task_awake(request->requester, NULL);
        continue;
        
        error:
            current_request_ready = false;
            current_request = NULL;
            request->code = -1;
            task_awake(request->requester, NULL);
    }
}

void disk_handler(int) {
    mutex_unlock(&state.disk_result);
}
