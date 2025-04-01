#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include "shell.h" // MAX_USER_INPUT
#include "shellmemory.h"
#include "pcb.h"

// Structure to track loaded scripts
struct LoadedScripts {
    char *filename;
    size_t line_base;
    size_t line_count;
    struct LoadedScripts *next;
};

static struct LoadedScripts *script_map = NULL;

// Check if a script is already loaded
static struct LoadedScripts *find_script(const char *filename) {
    struct LoadedScripts *current = script_map;
    while (current) {
        if (strcmp(current->filename, filename) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Register a newly loaded script
static void load_script(const char *filename, size_t base, size_t count) {
    struct LoadedScripts *new_script = malloc(sizeof(struct LoadedScripts));
    new_script->filename = strdup(filename);
    new_script->line_base = base;
    new_script->line_count = count;
    new_script->next = script_map;
    script_map = new_script;
}

void clear_script_map() {
    struct LoadedScripts *current = script_map;
    while (current) {
        struct LoadedScripts *next = current->next;
        free(current->filename);
        free(current);
        current = next;
    }
    script_map = NULL;
}

int pcb_has_next_instruction(struct PCB *pcb) {
    // have next if pc < line_count.
    // Sanity check: count = 0  ==> never have next. Good!
    return pcb->pc < pcb->line_count;
}

size_t pcb_next_instruction(struct PCB *pcb) {
    size_t i = pcb->line_base + pcb->pc;
    pcb->pc++;
    return i;
}

struct PCB *create_process(const char *filename) {
    // We have 2 main tasks:
    // load all the code in the script file into shellmemory, and
    // allocate+fill a PCB.
    struct LoadedScripts *existing = find_script(filename);
    struct PCB *pcb = malloc(sizeof(struct PCB));
    static pid fresh_pid = 1;
    pcb->pid = fresh_pid++;
    pcb->name = strdup(filename);
    pcb->next = NULL;
    pcb->pc = 0;

    if (existing) {
        pcb->line_base = existing->line_base;
        pcb->line_count = existing->line_count;
    } else {
        FILE *script = fopen(filename, "rt");
        if (!script) {
            perror("Failed to open file for create_process");
            free(pcb);
            return NULL;
        }
        pcb = create_process_from_FILE(script, filename);
        fclose(script);
    }
    pcb->duration = pcb->line_count;
    return pcb;
}

struct PCB *create_process_from_FILE(FILE *script, const char *filename) {
    // We can open the file, so we'll be making a process.
    struct PCB *pcb = malloc(sizeof(struct PCB));

    // The PID is the only weird part. They need to be distinct,
    // so let's use a static counter.
    static pid fresh_pid = 1;
    pcb->pid = fresh_pid++;

    // name should be the empty string, according to doc comment.
    pcb->name = strdup(filename);
    // next should be NULL, according to doc comment.
    pcb->next = NULL;

    // pc is always initially 0.
    pcb->pc = 0;

    // create initial values for base and count, in case we fail to read
    // any lines from the file. That way we'll end up with an empty process
    // that terminates as soon as it is scheduled -- reasonable behavior for
    // an empty script.
    // If we don't do this, and the loop below never runs, base would be
    // unitialized, which would be catastrophic.
    pcb->line_count = 0;
    pcb->line_base = 0;

    // We're told to assume lines of files are limited to 100 characters.
    // That's all well and good, but for implementing # we need to read
    // actual user input, and _that_ is limited to 1000 characters.
    // It's unclear if we should assume it's also limited to 100 for this
    // purpose. If you did assume that, that's OK! We didn't.
    char linebuf[MAX_USER_INPUT];
    while (!feof(script)) {
        memset(linebuf, 0, sizeof(linebuf));
        fgets(linebuf, MAX_USER_INPUT, script);

        size_t index = allocate_line(linebuf);
        // If we've run out of memory, clean up the partially-allocated
        // pcb and return NULL.
        if (index == (size_t)(-1)) {
            free_pcb(pcb);
            return NULL;
        }

        if (pcb->line_count == 0) {
            // do this on the first iteration only.
            pcb->line_base = index;
        }
        pcb->line_count++;
    }

    load_script(filename, pcb->line_base, pcb->line_count);
    // duration should initially match line_count.
    pcb->duration = pcb->line_count;

    return pcb;
}

void free_pcb(struct PCB *pcb) {
    for (size_t ix = pcb->line_base; ix < pcb->line_base + pcb->line_count; ++ix) {
        free_line(ix);
    }
    // Free the process name, but only if it's not the empty string.
    // The empty name (for the shell input process) was not malloc'd.
    if (strcmp("", pcb->name)) {
        free(pcb->name);
    }
    free(pcb);
}
