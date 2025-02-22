#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pcb.h"

struct PCB pcb_table[MAX_PROCESSES];
int next_pid = 0;

void pcb_init () {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        pcb_table[i].pid = -1;  // -1 means unused
    }
}

int pcb_create (int start_index, int length) {
    if (next_pid >= MAX_PROCESSES) {
        return -1;              // No more space for new processes
    }

    pcb_table[next_pid].pid = next_pid;
    pcb_table[next_pid].start = start_index;
    pcb_table[next_pid].length = length;
    pcb_table[next_pid].program_counter = 0;

    return next_pid++;
}

struct PCB *pcb_get (int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES || pcb_table[pid].pid == -1) {
        return NULL;            // Invalid PID
    }
    return &pcb_table[pid];
}

void pcb_update_program_counter (int pid, int new_pc) {
    struct PCB *pcb = pcb_get (pid);
    if (pcb != NULL) {
        pcb->program_counter = new_pc;
    }
}
