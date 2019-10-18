/* Implementation of simple command-line interface */

/* Each command defined in terms of a function */
typedef bool (*cmd_function)(int argc, char* argv[]);

/*Information about each command */
/* Organized as linked list in alphabetical order */
typedef struct CELE cmd_ele, *cmd_ptr;

struct CELE
{
    char* name;
    cmd_function operation;
    char *documentation;
    cmd_ptr next;
};

/* Initialize interpreter */
void init_cmd();

/* Add a new command */
void add_cmd(char* name, cmd_function operation, char *documentation);

/*Execute a command from a command line */
bool interpret_cmd(char *cmdline);

/*Execute a sequence of commands read from a file*/
bool interpret_file(FILE *fp);

/* Extract interger from text and store at loc */
bool get_int(char *vname, int *loc);

/* Add function to be executed as part of program exit */
void add_quit_helper(cmd_function qf);

/* Set propmt */
void set_prompt(char* prompt);

/* Turn echoing on/off */
void set_echo(bool on);





