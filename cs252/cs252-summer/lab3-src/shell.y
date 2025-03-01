%{
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <regex.h>
#include "command.h"
#include "single_command.h"
#include "shell.h"

void yyerror(const char* s);
int yylex();
void process_wildcards(char* prefix, char* suffix);
void expand_argument(char* arg);
int compare_strings(const void* a, const void* b);

#define MAX_FILENAME 1024
#define INITIAL_ENTRIES 20

char** file_entries;
int entry_count = 0;
int max_entries = INITIAL_ENTRIES;

%}

%union {
    char* str_val;
}

%token <str_val> WORD
%token PIPE NEWLINE STDOUT APPEND_STDOUT STDERR AMPERSAND STDIN STDOUT_STDERR APPEND_STDOUT_STDERR

%type <str_val> argument

%%

shell_input: command_sequence;

command_sequence:
    command_sequence complete_command
  | complete_command
;

complete_command:
    pipeline io_redirections background_opt NEWLINE {
        execute_command(g_current_command);
        g_current_command = malloc(sizeof(command_t));
        create_command(g_current_command);
    }
  | NEWLINE { print_prompt(); }
;

pipeline:
    pipeline PIPE cmd
  | cmd
;

cmd:
    WORD argument_list {
        g_current_single_command = malloc(sizeof(single_command_t));
        create_single_command(g_current_single_command);
        insert_argument(g_current_single_command, strdup($1));
        insert_single_command(g_current_command, g_current_single_command);
        g_current_single_command = NULL;
    }
;

argument_list:
    argument_list argument
  | /* empty */
;

argument:
    WORD {
        if (g_current_single_command->arguments[0] && strcmp(g_current_single_command->arguments[0], "echo") == 0 && strchr($1, '?')) {
            insert_argument(g_current_single_command, strdup($1));
        } else {
            expand_argument(strdup($1));
        }
    }
;

io_redirections:
    io_redirections io_redirection
  | /* empty */
;

io_redirection:
    STDOUT WORD {
        if (!g_current_command->out_file) {
            g_current_command->out_file = strdup($2);
        } else {
            fprintf(stderr, "Ambiguous output redirect.\n");
            exit(1);
        }
    }
  | APPEND_STDOUT WORD {
        if (!g_current_command->out_file) {
            g_current_command->out_file = strdup($2);
            g_current_command->append_out = 1;
        } else {
            fprintf(stderr, "Ambiguous output redirect.\n");
            exit(1);
        }
    }
  | STDERR WORD {
        if (!g_current_command->err_file) {
            g_current_command->err_file = strdup($2);
        } else {
            fprintf(stderr, "Ambiguous error redirect.\n");
            exit(1);
        }
    }
  | STDOUT_STDERR WORD {
        if (!g_current_command->out_file && !g_current_command->err_file) {
            g_current_command->out_file = strdup($2);
            g_current_command->err_file = strdup($2);
        } else {
            fprintf(stderr, "Ambiguous output/error redirect.\n");
            exit(1);
        }
    }
  | APPEND_STDOUT_STDERR WORD {
        if (!g_current_command->out_file && !g_current_command->err_file) {
            g_current_command->out_file = strdup($2);
            g_current_command->err_file = strdup($2);
            g_current_command->append_out = 1;
            g_current_command->append_err = 1;
        } else {
            fprintf(stderr, "Ambiguous output/error redirect.\n");
            exit(1);
        }
    }
  | STDIN WORD {
        g_current_command->in_file = strdup($2);
    }
;

background_opt:
    AMPERSAND { g_current_command->background = 1; }
  | /* empty */
;

%%

void yyerror(const char* s) {
    fprintf(stderr, "%s\n", s);
}

int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void expand_argument(char* arg) {
    entry_count = 0;
    file_entries = malloc(max_entries * sizeof(char*));

    if (strchr(arg, '*') || strchr(arg, '?')) {
        process_wildcards("", arg);
        qsort(file_entries, entry_count, sizeof(char*), compare_strings);
        for (int i = 0; i < entry_count; i++) {
            insert_argument(g_current_single_command, file_entries[i]);
        }
    } else {
        insert_argument(g_current_single_command, arg);
    }
}

void process_wildcards(char* prefix, char* suffix) {
    if (*suffix == '\0') {
        if (entry_count == max_entries) {
            max_entries *= 2;
            file_entries = realloc(file_entries, max_entries * sizeof(char*));
        }
        file_entries[entry_count++] = strdup(prefix);
        return;
    }

    char* slash = strchr(suffix, '/');
    char component[MAX_FILENAME];
    if (slash) {
        strncpy(component, suffix, slash - suffix);
        component[slash - suffix] = '\0';
        suffix = slash + 1;
    } else {
        strcpy(component, suffix);
        suffix += strlen(suffix);
    }

    char new_prefix[MAX_FILENAME];
    if (!strchr(component, '*') && !strchr(component, '?')) {
        snprintf(new_prefix, MAX_FILENAME, "%s%s%s", prefix, (*prefix ? "/" : ""), component);
        process_wildcards(new_prefix, suffix);
        return;
    }

    char* regex = malloc(2 * strlen(component) + 3);
    char* r = regex;
    *r++ = '^';
    for (char* c = component; *c; c++) {
        if (*c == '*') { *r++ = '.'; *r++ = '*'; }
        else if (*c == '?') { *r++ = '.'; }
        else if (*c == '.') { *r++ = '\\'; *r++ = '.'; }
        else { *r++ = *c; }
    }
    *r++ = '$';
    *r = '\0';

    regex_t re;
    if (regcomp(&re, regex, REG_EXTENDED | REG_NOSUB) != 0) {
        perror("regcomp");
        free(regex);
        return;
    }

    DIR* dir = opendir(*prefix ? prefix : ".");
    if (!dir) {
        if (!suffix) {
            if (entry_count == max_entries) {
                max_entries *= 2;
                file_entries = realloc(file_entries, max_entries * sizeof(char*));
            }
            file_entries[entry_count++] = strdup(prefix);
        }
        regfree(&re);
        free(regex);
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (regexec(&re, entry->d_name, 0, NULL, 0) == 0) {
            if ((component[0] == '.' && entry->d_name[0] == '.') ||
                (component[0] != '.' && entry->d_name[0] != '.')) {
                snprintf(new_prefix, MAX_FILENAME, "%s%s%s", 
                         prefix, (*prefix && strcmp(prefix, "/") ? "/" : ""), entry->d_name);
                process_wildcards(new_prefix, suffix);
            }
        }
    }

    closedir(dir);
    regfree(&re);
    free(regex);
}
