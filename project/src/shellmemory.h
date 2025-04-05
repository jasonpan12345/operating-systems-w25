#ifdef MEM_SIZE
#else
#   define MEM_SIZE 999
#endif

#ifdef VAR_SIZE
#else
#   define VAR_SIZE 999
#endif

#define FRAME_SIZE 3
#define MAX_FRAMES (MEM_SIZE / FRAME_SIZE)

void assert_linememory_is_empty (void);
void free_line (size_t index);
const char *get_line (size_t index);
void reset_linememory_allocator (void);

void mem_init ();
void init_frame_map ();
char *mem_get_value (char *var);
void mem_set_value (char *var, char *value);

size_t find_free_frame ();
void load_frame_line (const char *line, size_t index);
int line_allocated (size_t index);
void map_frame_to_pt (size_t frame, size_t *page_table);
