#ifndef PCB_H
#   define PCB_H

#   define MAX_PROCESSES 100

struct PCB {
    int pid;
    int start;                  // start index
    int length;
    int program_counter;
    struct PCB *next;
};

extern struct PCB pcb_table[MAX_PROCESSES];
extern int next_pid;

void pcb_init ();
int pcb_create (int start, int length);
struct PCB *pcb_get (int pid);
void pcb_update_program_counter (int pid, int new_pc);

#endif
