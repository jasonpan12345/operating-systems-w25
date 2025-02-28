#include <stdio.h>
#include "scheduler.h"
#include "interpreter.h"
#include "shellmemory.h"
#include "shell.h"

void scheduler_init () {
    ready_queue_init ();        // Initialize ready queue
}

void scheduler_run () {
    while (!ready_queue_is_empty ()) {
        // Get process at head of ready queue
        struct PCB *pcb = ready_queue_remove ();
        if (pcb == NULL) {      // no process in ready queue
            return;
        }
        // Execute process
        while (pcb->program_counter < pcb->length) {
            // Fetch current instruction
            char *line =
                mem_get_program_line (pcb->start + pcb->program_counter);
            if (line == NULL) { // invalid program line
                break;
            }
            parseInput (line);
            // Update program counter
            pcb_update_program_counter (pcb->pid, pcb->program_counter + 1);
        }

        // Cleanup process after finishing execution
        scheduler_cleanup (pcb);
    }
}

void scheduler_cleanup (struct PCB *pcb) {
    // Free the program lines associated with the process
    for (int i = pcb->start; i < pcb->start + pcb->length; i++) {
        mem_clean_program_line (i);
    }

    // Mark the PCB as unused
    pcb->pid = -1;
    pcb->start = -1;
    pcb->length = -1;
    pcb->program_counter = -1;
    pcb->next = NULL;
}
