#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>

int MAX_ARGS_SIZE = 3;

int badcommand () {
    printf ("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist () {
    printf ("Bad command: File not found\n");
    return 3;
}

int help ();
int quit ();
int set (char *var, char *value);
int print (char *var);
int source (char *script);
int badcommandFileDoesNotExist ();

int echo (char *var);
int my_ls ();
int my_mkdir (char *var);
int my_touch (char *var);
int my_cd (char *var);
int run (char *command_args[], int args_size);

// Interpret commands and their arguments
int interpreter (char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand ();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn (command_args[i], "\r\n")] = 0;
    }

    if (strcmp (command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand ();
        return help ();

    } else if (strcmp (command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand ();
        return quit ();

    } else if (strcmp (command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand ();
        return set (command_args[1], command_args[2]);

    } else if (strcmp (command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand ();
        return print (command_args[1]);

    } else if (strcmp (command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand ();
        return source (command_args[1]);

    } else if (strcmp (command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand ();
        return echo (command_args[1]);
    } else if (strcmp (command_args[0], "my_ls") == 0) {
        if (args_size != 1)
            return badcommand ();
        return my_ls (command_args[1]);
    } else if (strcmp (command_args[0], "my_mkdir") == 0) {
        if (args_size != 2)
            return badcommand ();
        return my_mkdir (command_args[1]);
    } else if (strcmp (command_args[0], "my_touch") == 0) {
        if (args_size != 2)
            return badcommand ();
        return my_touch (command_args[1]);
    } else if (strcmp (command_args[0], "my_cd") == 0) {
        if (args_size != 2)
            return badcommand ();
        return my_cd (command_args[1]);
    } else if (strcmp (command_args[0], "run") == 0) {
        if (args_size < 2)
            return badcommand ();
        return run (command_args + 1, args_size - 1);
    } else
        return badcommand ();
}

int help () {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
echo STRING / $VAR     Displays STRING / Displays the STRING assigned to VAR\n \
my_ls                   Lists all the files present in the current directory\n \
my_mkdir                creates a new directory with the name dirname in the current directory\n \
my_touch                creates a new empty file inside the current directory\n \
my_cd                   changes current directory to directory dirname\n \
run ARGS                executes given command\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf ("%s\n", help_string);
    return 0;
}

int quit () {
    printf ("Bye!\n");
    exit (0);
}

int set (char *var, char *value) {
    char *link = " ";

    // Hint: If "value" contains multiple tokens, you'll need to loop through them, 
    // concatenate each token to the buffer, and handle spacing appropriately. 
    // Investigate how `strcat` works and how you can use it effectively here.

    mem_set_value (var, value);
    return 0;
}


int print (char *var) {
    printf ("%s\n", mem_get_value (var));
    return 0;
}

int source (char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen (script, "rt");     // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist ();
    }

    fgets (line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput (line);    // which calls interpreter()
        memset (line, 0, sizeof (line));

        if (feof (p)) {
            break;
        }
        fgets (line, MAX_USER_INPUT - 1, p);
    }

    fclose (p);

    return errCode;
}

int echo (char *var) {
    if (var[0] == '$') {        // check if first character is '$'
        if (strcmp (mem_get_value (var + 1), "Variable does not exist") == 0) {
            printf ("\n");      // variable does not exist
        } else {
            printf ("%s\n", mem_get_value (var + 1));   // variable exists
        }
    } else {
        printf ("%s\n", var);
    }

    return 0;
}

int alphasort (const struct dirent **a, const struct dirent **b) {
    return strcoll ((*a)->d_name, (*b)->d_name);
}

int my_ls () {
    struct dirent **namelist;
    int n;

    // Scan directory and sort alphabetically
    n = scandir (".", &namelist, NULL, alphasort);
    if (n < 0) {
        return -1;
    }

    for (int i = 0; i < n; i++) {       // Print filenames
        printf ("%s\n", namelist[i]->d_name);
        free (namelist[i]);     // Free allocated dirent struct
    }
    free (namelist);

    return 0;
}

// Function to check if a string is alphanumeric
int isAlphanumeric (const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalnum (str[i])) {
            return 0;           // Not alphanumeric
        }
    }
    return 1;                   // Alphanumeric
}

int my_mkdir (char *var) {
    if (var[0] == '$') {        // check if first character is '$'
        if (strcmp (mem_get_value (var + 1), "Variable does not exist") == 0) {
            printf ("Bad command: my_mkdir\n"); // variable does not exist
        } else if (!isAlphanumeric (mem_get_value (var + 1))) {
            printf ("Bad command: my_mkdir\n");
        } else {
            mkdir (mem_get_value (var + 1), 0755);      // create directory with variable value
        }
    } else {
        if (!isAlphanumeric (var)) {
            printf ("Bad command: my_mkdir\n");
        }
        mkdir (var, 0755);
    }
    return 0;
}

int my_touch (char *filename) {
    FILE *file = fopen (filename, "w");
    fclose (file);
    return 0;
}

int my_cd (char *dirname) {
    DIR *dir = opendir (dirname);
    if (dir == NULL) {          // Directory does not exist
        printf ("Bad command: my_cd\n");
        return 1;
    } else {                    // Directory exists
        chdir (dirname);
        closedir (dir);
        return 0;
    }
}

int run (char *command_args[], int args_size) {
    fflush (stdout);            // Flush output before forking

    pid_t pid = fork ();
    if (pid < 0) {
        return 1;
    }
    if (pid == 0) {             // Child process
        command_args[args_size] = NULL;
        execvp (command_args[0], command_args);
        exit (1);
    } else {                    // Parent process
        int status;
        waitpid (pid, &status, 0);
        if (WIFEXITED (status)) {
            return WEXITSTATUS (status);
        } else {
            return 1;
        }
    }
}
