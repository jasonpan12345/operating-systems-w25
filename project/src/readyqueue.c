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

struct PCB *ready_queue_get_head () {
    return ready_queue.head;
}

void ready_queue_insert (struct PCB *pcb) { // insert function for AGING
    pcb->next = NULL;           // set next pointer to NULL

    if (ready_queue.head == NULL) {     // Queue is empty
        ready_queue.head = pcb; // set head and tail to the new PCB
        ready_queue.tail = pcb;
    } else {
        // insert PCB in the correct position based on job_length_score
        struct PCB *current = ready_queue.head;
        struct PCB *previous = NULL;

        while (current != NULL && current->job_length_score < pcb->job_length_score) {
            previous = current;
            current = current->next;
        }

        if (previous == NULL) { // Insert at the head of the queue
            pcb->next = ready_queue.head;
            ready_queue.head = pcb;
        } else {    // Insert after previous
            pcb->next = current;
            previous->next = pcb;
        }

        if (current == NULL) { // Update tail if inserting at the end
            ready_queue.tail = pcb;
        }
    }
}