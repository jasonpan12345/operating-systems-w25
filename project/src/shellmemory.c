#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct program_line {
    char *line;
};  

struct memory_struct shellmemory[MEM_SIZE];
struct program_line program_memory[MAX_PROGRAM_LINES];
int program_line_count = 0;

// Helper functions
int match (char *model, char *var) {
    int i, len = strlen (var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}

// Shell memory functions

void mem_init () {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
    for (i = 0; i < MAX_PROGRAM_LINES; i++) {
        program_memory[i].line = NULL;
    }
}

// Set key value pair
void mem_set_value (char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup (value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup (var_in);
            shellmemory[i].value = strdup (value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *mem_get_value (char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, var_in) == 0) {
            return strdup (shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}

int mem_store_program_line(char *line) {
    if (program_line_count >= MAX_PROGRAM_LINES) {
        return -1; // No space left
    }
    program_memory[program_line_count].line = strdup(line);
    program_line_count++;
    return program_line_count - 1;
}

char *mem_get_program_line(int index) {
    if (index < 0 || index >= program_line_count) {
        return NULL;
    }
    return program_memory[index].line;
}