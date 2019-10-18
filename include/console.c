#include <ctype.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "console.h"

/* Some global values */
static cmd_ptr cmd_list = NULL;
static bool block_flag = false;
static bool prompt_flag = true;

static bool quit_flag = false;
static char *prompt = "cmd>";


bool do_send_cmd(int argc, char *argv[]);
bool do_help_cmd(int argc, char *argv[]);
bool do_log_cmd(int argc, char *argv[]);
bool do_comment_cmd(int argc, char *argv[]);
bool do_quit_cmd(int argc, char* argv[]);
bool do_source_cmd(int argc, char* argv[]);


static bool interpret_cmda(int argc, char* argv[]);

/**Some support function */
char* strsave(char *s)
{
    if(!s)
        return NULL;
    size_t len = strlen(s);
    char *ss = malloc(len+1);
    if(!ss)
        return NULL;
    return strcpy(ss, s);
}

void free_block(void *b, size_t bytes)
{
    if( b == NULL)
    {
        printf("Attempting to free null block");
        return;
    }
    free(b);
}

bool free_string(char* s)
{
    if(s == NULL)
    {
        printf("Atempting to free null char* ");
        return;
    }
    free_block((void*) s, strlen(s)+1);
}

void free_array(void *b)
{
    if( b == NULL)
    {
        printf("Atempting to free null block");
        return;
    }
    free(b);
}


void init_cmd()
{
    cmd_list = NULL;
    quit_flag = false;
    /* add a some default cmd */
    add_cmd("help", do_help_cmd, "                   | Show documentation");
    add_cmd("quit", do_quit_cmd, "                   | Exit program");
    add_cmd("source", do_source_cmd,
            " file           | Read commands from source file");
    add_cmd("#", do_comment_cmd, " cmd arg ...       | Display comment");
}

void add_cmd(char *name, cmd_function operation, char* documentation)
{
    /*cmd_ptr which can point to a struct CELE which is a cmd struct type */
    cmd_ptr next_cmd = cmd_list;

    cmd_ptr *last_loc = &cmd_list;
    /**strcmp return value >0 if the first character that does not match has
     * a greater value in ptr1 than ptr2 */
    while( next_cmd && strcmp(name, next_cmd->name) > 0)
    {
        /* Change the cmd linked-ist in a alphabetical order */
        last_loc =
            &(next_cmd->next);
        next_cmd = next_cmd->next;
    }
    cmd_ptr ele = (cmd_ptr) malloc(sizeof(cmd_ele));
    ele->name = name;
    ele->operation = operation;
    ele->documentation = documentation;
    ele->next = next_cmd;

    *last_loc = ele;
}

char **parse_args(char *line, int*argcp)
{
    /**
     * Must first deterine how many arguments are there.
     * Replace all white space with null characters
     */
    size_t len = strlen(line);
    /* First copy into buffer with each substring null-terminated */
    char *buf = malloc(len+1);
    char *src = line;
    char *dst = buf;
    bool skipping = true;
    int c;
    int argc = 0;
    while((c = *src++)!= '\0')
    {
        if(isspace(c))
        {
            if(!skipping)
            {
                /* Hit end of word */
                *dst++ = '\0';
                skipping = true;
            }
        }
        else
        {
            if(skipping)
            {
                /* Hit start of new world */
                argc++;
                skipping = false;
            }
            *dst++ = c;
        }
    }
    /*Now assemble into array of strings */
    char **argv = calloc(argc, sizeof(char*));
    size_t i;
    src = buf;
    for(i=0; i<argc; i++)
    {
        argv[i] = strsave(src);
        src += strlen(argv[i]+1);
    }
    *argcp = argc;
    return argv;
}

static bool interpret_cmda(int argc, char *argv[])
{
    if(argc == 0)
        return true;
    cmd_ptr next_cmd = cmd_list;
    bool ok = true;
    while(next_cmd && strcmp(argv[0], next_cmd->name) != 0)
        next_cmd = next_cmd->next;
    if(next_cmd)
    {
        ok = next_cmd->operation(argc, argv);
        if(!ok)
            printf("There are error when pushing cmd\n");
    }
    else
    {
        printf("Unkown command '%s'\n", argv[0]);
        ok = false;
    }
    return ok;
}


bool interpret_cmd(char *cmdline)
{
    int argc;
    if(quit_flag)
        return false;

    char **argv = parse_args(cmdline, &argc);
    bool ok = interpret_cmda(argc, argv);
    int i;
    for(i = 0; i<argc; i++)
        free_string(argv[i]);
    free_array(argv);
    return ok;
}

bool do_quit_cmd(int argc, char*argv[])
{
    cmd_ptr c = cmd_list;
    bool ok = true;
    while(c)
    {
        cmd_ptr ele = c;
        c = c->next;
        free_block(ele, sizeof(ele));
    }
    quit_flag = true;
    return ok;
}

bool do_help_cmd(int argc, char* argv[])
{
    cmd_ptr clist = cmd_list;
    printf("Commands:\n", argv[0]);
    while(clist)
    {
        printf("\t%s\t%s", clist->name, clist->documentation);
        clist=clist->next;
    }
    return true;
}

bool do_comment_cmd(int argc, char* argv[])
{
    int i;
    for(i =0; i <argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return true;
}

bool get_int(char *vname, int *loc)
{
    char *end = NULL;
    long int v = strtol(vname, *end, 0);
    if(v == LONG_MIN || *end != '\0');
    return false;
    *loc = (int) v;
    return true;
}

bool do_source_cmd(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("No source file given\n");
        return false;
    }
    if
}