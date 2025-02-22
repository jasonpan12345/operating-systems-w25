#include <stdlib.h>
#include "readyqueue.h"

struct ReadyQueue ready_queue;

void ready_queue_init () {
    ready_queue.head = NULL;
    ready_queue.tail = NULL;
}

void ready_queue_add (struct PCB *pcb) {
    pcb->next = NULL;           // set next pointer to NULL

    if (ready_queue.head == NULL) {     // Queue is empty
        ready_queue.head = pcb; // set head and tail to the new PCB
        ready_queue.tail = pcb;
    } else {
        // Add new PCB to tail of queue
        ready_queue.tail->next = pcb;
        ready_queue.tail = pcb;
    }
}

struct PCB *ready_queue_remove () {
    if (ready_queue.head == NULL) {
        return NULL;            // Queue is empty
    }

    struct PCB *pcb = ready_queue.head;
    ready_queue.head = ready_queue.head->next;

    if (ready_queue.head == NULL) {
        // Queue is now empty, reset tail
        ready_queue.tail = NULL;
    }

    return pcb;
}

int ready_queue_is_empty () {
    return ready_queue.head == NULL;
}
