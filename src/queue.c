#include<assert.h>
#include<stdio.h>

#include"queue.h"

int queue_size (queue_t *queue) {
    int count = 0;
    queue_t *current = queue;
    while (current != NULL) {
        count++;
        current = current->next;
        if (current == queue)
            break;
    }
    return count;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    queue_t *current = queue;
    
    printf("%s", name);
    while (current != NULL) {
        print_elem(current);
        printf(" ");
        current = current->next;
        if (current == queue)
            break;
    }
    printf("\n");
}

int queue_append (queue_t **queue, queue_t *elem) {
    if (elem->next != NULL || elem-> prev != NULL) {
        fprintf(stderr, "ERROR: elem already in a queue\n");
        return -1;
    }

    if (*queue == NULL) {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return 0;
    }
    elem->next = *queue;
    elem->prev = (*queue)->prev;
    elem->prev->next = elem;
    (*queue)->prev = elem;
    return 0;
}

int queue_contains(queue_t *queue, queue_t *elem) {
    queue_t *current = queue;
    while (current != NULL) {
        if (current == elem)
            return 1;
        current = current->next;
        if (current == queue)
            break;
    }
    
    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem) {
    if (*queue == NULL) {
        fprintf(stderr, "ERROR: queue empty\n");
        return -1;
    }

    if (elem->prev == NULL || elem->next == NULL) {
        fprintf(stderr, "ERROR: elem is not in the queue\n");
        return -1;
    }

    if (!queue_contains(*queue, elem)) {
        return -1;
    }

    if (*queue == elem) {
        if (elem->next == elem)
            *queue = NULL;
        else
            *queue = elem->next;
    }
    elem->next->prev = elem->prev;
    elem->prev->next = elem->next;

    elem->prev = NULL;
    elem->next = NULL;

    return 0;
}
