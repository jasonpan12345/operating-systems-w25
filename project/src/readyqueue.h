#include "pcb.h"

struct ReadyQueue {
    struct PCB *head;
    struct PCB *tail;
};

void ready_queue_init ();
void ready_queue_add (struct PCB *pcb);
struct PCB *ready_queue_remove ();
int ready_queue_is_empty ();
struct PCB *ready_queue_get_head ();
void ready_queue_insert (struct PCB *pcb);