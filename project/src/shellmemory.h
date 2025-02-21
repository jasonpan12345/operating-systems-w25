#define MEM_SIZE 1000
#define MAX_PROGRAM_LINES 1000
void mem_init ();
char *mem_get_value (char *var);
void mem_set_value (char *var, char *value);

int mem_store_program_line(char *line);
char *mem_get_program_line(int index);