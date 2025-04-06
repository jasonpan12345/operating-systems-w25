#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
#include "pcb.h"

#define true 1
#define false 0

// ---------------------
// Line memory for program lines; for part 2.
// ---------------------

// We know that program lines will be read, but not modified, until they are
// removed from the line memory. Therefore, we can provide the line directly
// to requests, rather than copying it. Freeing a program will free all of its
// lines.
// When a line is saved, it should be copied. This way, it can be saved from
// a stack-allocated buffer without fear of losing it when the buffer is freed.
struct program_line {
    int allocated;              // for sanity-checking
    char *line;
};

struct program_line linememory[MEM_SIZE];
size_t *frame_map[MAX_FRAMES];
static size_t LRU_table[MAX_FRAMES];    // The greater, the older

size_t next_free_line = 0;
// So we simply record the next free index in a variable.
// We can't reset this whenever a PCB is freed, because we might be running
// with # and the shell might be about to allocate another program and that
// program might not fit in the first hole! So we provide a function to reset
// this, which should be called whenever `exec` is called and we're not in
// 'background mode.'
void reset_linememory_allocator () {
    next_free_line = 0;
    assert_linememory_is_empty ();
}

// The comments above detail an important, nontrivial invariant, that the
// linememory is completely empty at certain times.
// For sanity checking, we provide an additional function to assert that the
// the linememory is empty.

void assert_linememory_is_empty () {
    for (size_t i = 0; i < MEM_SIZE; ++i) {
        assert (!linememory[i].allocated);
        assert (linememory[i].line == NULL);
    }
}

// note that init_linemem is not exposed from the header.
// We made init_mem call init_linemem, down below.
void init_linemem () {
    for (size_t i = 0; i < MEM_SIZE; ++i) {
        linememory[i].allocated = false;
        linememory[i].line = NULL;
    }
}

void init_frame_map () {
    for (size_t i = 0; i < MAX_FRAMES; i++) {
        frame_map[i] = NULL;
    }
}

int line_allocated (size_t index) {
    return linememory[index].allocated;
}

size_t find_free_frame () {
    // search through entire linememory by steps of FRAME_SIZE
    for (size_t i = 0; i < MEM_SIZE; i += FRAME_SIZE) {
        if (!linememory[i].allocated) { // found unallocated frame
            return i;
        }
    }
    return (size_t) (-1);
}

void load_frame_line (const char *line, size_t index) {
    if (index >= MEM_SIZE) {
        return;                 // Safety check        
    }
    if (linememory[index].allocated) {
        free_line (index);      // Clean up if already allocated
    }

    linememory[index].allocated = true;
    linememory[index].line = strdup (line);
    reset_frame_lru (index / FRAME_SIZE);
}

void map_frame_to_pt (size_t frame, size_t *page_table) {
    if (frame < MEM_SIZE) {     // Ensure frame is valid
        frame_map[frame / FRAME_SIZE] = page_table;
    }
}

// To free a line, we must deallocate it and mark it unused.
// We don't mess with next_free_line for the reasons described above.
void free_line (size_t index) {
    if (linememory[index].allocated) {
        free (linememory[index].line);
        linememory[index].allocated = false;
        linememory[index].line = NULL;
    }
}

// Return a const pointer to ensure the caller doesn't do something horrific,
// like try to free it.
const char *get_line (size_t index) {
    assert (linememory[index].allocated);
    update_lru_table ();
    reset_frame_lru (index / FRAME_SIZE);
    return linememory[index].line;
}

// [Note: OBS]
// Thinking about memory in terms of "owners" and "borrowers" was
// popularized by Rust, and recently formalized by so-called
// Ownership and Borrowing Systems. They are a concrete way to ensure that our
// programs do not leak memory or access invalid memory, at the cost of
// potentially copying data sometimes that it is not necessary to.
// A small price to pay!
// We highly recommend looking into how this works and using it as
// a mental model when you program.

// --------------------- 
// Key-value memory for variables; from part 1.
// ---------------------

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

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
        // Include a character that's impossible to type so that
        // we don't mistake an empty entry for an entry named 'none'.
        shellmemory[i].var = "none\1";
        shellmemory[i].value = "none";
    }

    init_linemem ();
    init_frame_map ();
    // Initialize LRU_table
    for (size_t i = 0; i < MAX_FRAMES; i++) {
        LRU_table[i] = 0;
    }
}

// Set key value pair
void mem_set_value (char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, var_in) == 0) {
            free (shellmemory[i].value);
            shellmemory[i].value = strdup (value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, "none\1") == 0) {
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
    return NULL;
}

size_t find_lru_frame () {
    size_t oldest_time = 0;
    size_t lru_frame = 0;

    for (size_t i = 0; i < MAX_FRAMES; i++) {
        //printf("LRU_table[%zu] = %zu\n",i, LRU_table[i]);
        if (LRU_table[i] > oldest_time && frame_map[i] != NULL) {
            oldest_time = LRU_table[i];
            lru_frame = i;
        }
    }
    return lru_frame;
}

// Add 1 time period to every frame
void update_lru_table () {
    for (size_t i = 0; i < MAX_FRAMES; i++) {
        if (frame_map[i] != NULL) {
            LRU_table[i]++;     // Only update frames in use
        }
    }
}

void reset_frame_lru (size_t frame) {
    LRU_table[frame] = 0;
}
