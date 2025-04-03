#include <stdio.h>
#include <stdlib.h>
#include <string.h>             // memset
#include "shell.h"              // MAX_USER_INPUT
#include "shellmemory.h"
#include "pcb.h"

// Structure to track loaded scripts
struct LoadedScripts {
    char *filename;
    size_t page_count;
    size_t *page_table;
    struct LoadedScripts *next;
};

static struct LoadedScripts *script_map = NULL;

// Check if a script is already loaded
static struct LoadedScripts *find_script (const char *filename) {
    struct LoadedScripts *current = script_map;
    while (current) {
        if (strcmp (current->filename, filename) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Register a newly loaded script
static void load_script (const char *filename, size_t *page_table,
                         size_t page_count) {
    struct LoadedScripts *new_script = malloc (sizeof (struct LoadedScripts));
    new_script->filename = strdup (filename);
    new_script->page_table = page_table;
    new_script->page_count = page_count;
    new_script->next = script_map;      // append existing script_map to new_script
    script_map = new_script;
}

void clear_script_map () {
    struct LoadedScripts *current = script_map;
    while (current) {
        struct LoadedScripts *next = current->next;
        free (current->filename);
        free (current);
        current = next;
    }
    script_map = NULL;
}

int pcb_has_next_instruction (struct PCB *pcb) {
    // have next if pc < line_count.
    // Sanity check: count = 0  ==> never have next. Good!
    return pcb->pc < pcb->line_count;
}

size_t pcb_next_instruction (struct PCB *pcb) {
    size_t virtual_line = pcb->pc;
    size_t page = virtual_line / FRAME_SIZE;
    size_t offset = virtual_line % FRAME_SIZE;

    if (page >= pcb->page_count) {
        return (size_t) -1;     // Invalid instruction
    }

    size_t frame = pcb->page_table[page];
    pcb->pc++;
    return frame + offset;
}

static pid fresh_pid = 1;

struct PCB *create_process (const char *filename) {
    // We have 2 main tasks:
    // load all the code in the script file into shellmemory, and
    // allocate+fill a PCB.
    struct LoadedScripts *existing = find_script (filename);
    struct PCB *pcb = malloc (sizeof (struct PCB));


    if (existing) {
        pcb->pid = fresh_pid++;
        pcb->name = strdup (filename);
        pcb->next = NULL;
        pcb->pc = 0;
        //pcb->line_base = existing->line_base;
        pcb->page_count = existing->page_count;
        pcb->page_table = existing->page_table;
    } else {
        FILE *script = fopen (filename, "rt");
        if (!script) {
            perror ("Failed to open file for create_process");
            free (pcb);
            return NULL;
        }
        pcb = create_process_from_FILE (script, filename);
        fclose (script);
    }
    pcb->duration = pcb->line_count;
    return pcb;
}

struct PCB *create_process_from_FILE (FILE * script, const char *filename) {
    // We can open the file, so we'll be making a process.
    struct PCB *pcb = malloc (sizeof (struct PCB));

    pcb->pid = fresh_pid++;

    // name should be the empty string, according to doc comment.
    pcb->name = strdup ("");
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

    size_t line_count = 0;

    // Count lines first to determine page count
    char linebuf[MAX_USER_INPUT];
    while (!feof (script)) {
        memset (linebuf, 0, sizeof (linebuf));
        fgets (linebuf, MAX_USER_INPUT, script);
        if (linebuf[0] != '\0')
            line_count++;
    }
    rewind (script);

    size_t page_count = (line_count + FRAME_SIZE - 1) / FRAME_SIZE;
    size_t *page_table = malloc (page_count * sizeof (size_t));
    pcb->page_count = page_count;
    pcb->page_table = page_table;
    pcb->line_count = line_count;

    size_t index;
    size_t current_page = 0;
    size_t frame_line_count = 0;

    while (!feof (script)) {
        memset (linebuf, 0, sizeof (linebuf));
        fgets (linebuf, MAX_USER_INPUT, script);

        if (frame_line_count == 0) {    // starting from new page
            index = find_free_frame ();

            // If we've run out of memory, clean up the partially-allocated
            // pcb and return NULL.
            if (index == (size_t) (-1)) {
                free_pcb (pcb);
                fclose (script);
                return NULL;
            }
            pcb->page_table[current_page] = index;      // map page to line frame store
        }

        load_frame_line (linebuf, (index + frame_line_count));  // load line into frame (base + offset)
        frame_line_count++;

        if (frame_line_count == FRAME_SIZE) {   // exceeded current frame size, need new frame
            current_page++;
            frame_line_count = 0;
        }

    }

    load_script (filename, pcb->page_table, pcb->page_count);
    // duration should initially match line_count.
    pcb->duration = pcb->line_count;

    return pcb;
}

void free_pcb (struct PCB *pcb) {
    // Only free frames if this is the original process
    if (strcmp (pcb->name, "") != 0) {
        struct LoadedScripts *script = find_script (pcb->name);
        if (script && script->page_table == pcb->page_table) {
            // Original process - free frames and page table
            for (size_t i = 0; i < pcb->page_count; i++) {
                for (size_t offset = 0; offset < FRAME_SIZE; offset++) {
                    size_t index = pcb->page_table[i];
                    free_line (index + offset);
                }
            }
            free (pcb->page_table);
        }
    }

    if (strcmp ("", pcb->name)) {
        free (pcb->name);
    }
    free (pcb);
}
