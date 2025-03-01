/*
 * shell.y: Parser for shell
 * Updated to remove the 'executable' field and fix double free errors
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "single_command.h"
#include "shell.h"

void yyerror(const char * s);
int yylex();

int redirection_error = 0;

%}

%union {
    char *string;
}

%token <string> WORD
%token PIPE AMPERSAND GREAT GREATAMPERSAND GREATGREAT GREATGREATAMPERSAND LESS TWOGREAT NEWLINE

%start goal

%%

goal:
      /* Empty */
    | goal line
    ;

line:
      NEWLINE
        {
            print_prompt();
        }
    | command NEWLINE
        {
            if (redirection_error) {
                /* Clear the command and prepare for next input */
                redirection_error = 0;
                free_command(g_current_command);
                create_command(g_current_command);
                print_prompt();
            } else {
                execute_command(g_current_command);
                /* Prepare for next command */
                free_command(g_current_command);
                create_command(g_current_command);
            }
        }
    | error NEWLINE
        {
            yyerror("Syntax error");
            /* Discard input until next newline */
            yyerrok;
            print_prompt();
        }
    ;

command:
      simple_command_list io_modifier_list background_opt
    ;

simple_command_list:
      simple_command
        {
            insert_single_command(g_current_command, g_current_single_command);
        }
    | simple_command_list PIPE simple_command
        {
            insert_single_command(g_current_command, g_current_single_command);
        }
    ;

simple_command:
      command_word argument_list
    ;

command_word:
      WORD
        {
            /* Handle 'exit' command */
            if (strcmp($1, "exit") == 0) {
                exit(0);
            }
            /* Create new single command */
            g_current_single_command = (single_command_t *)malloc(sizeof(single_command_t));
            create_single_command(g_current_single_command);
            insert_argument(g_current_single_command, $1);
        }
    ;

argument_list:
      /* Empty */
    | argument_list WORD
        {
            insert_argument(g_current_single_command, $2);
        }
    ;

io_modifier_list:
      /* Empty */
    | io_modifier_list io_modifier
    ;

io_modifier:
      GREAT WORD
        {
            if (g_current_command->out_file != NULL) {
                fprintf(stderr, "Ambiguous output redirect.\n");
                redirection_error = 1;
            } else {
                g_current_command->out_file = $2;
                g_current_command->append_out = false;
            }
        }
    | GREATGREAT WORD
        {
            if (g_current_command->out_file != NULL) {
                fprintf(stderr, "Ambiguous output redirect.\n");
                redirection_error = 1;
            } else {
                g_current_command->out_file = $2;
                g_current_command->append_out = true;
            }
        }
    | LESS WORD
        {
            if (g_current_command->in_file != NULL) {
                fprintf(stderr, "Ambiguous input redirect.\n");
                redirection_error = 1;
            } else {
                g_current_command->in_file = $2;
            }
        }
    | TWOGREAT WORD
        {
            if (g_current_command->err_file != NULL) {
                fprintf(stderr, "Ambiguous error redirect.\n");
                redirection_error = 1;
            } else {
                g_current_command->err_file = $2;
                g_current_command->append_err = false;
            }
        }
    | GREATAMPERSAND WORD
        {
            if (g_current_command->out_file != NULL || g_current_command->err_file != NULL) {
                fprintf(stderr, "Ambiguous output redirect.\n");
                redirection_error = 1;
            } else {
                g_current_command->out_file = $2;
                g_current_command->err_file = strdup($2);
                g_current_command->append_out = false;
                g_current_command->append_err = false;
            }
        }
    | GREATGREATAMPERSAND WORD
        {
            if (g_current_command->out_file != NULL || g_current_command->err_file != NULL) {
                fprintf(stderr, "Ambiguous output redirect.\n");
                redirection_error = 1;
            } else {
                g_current_command->out_file = $2;
                g_current_command->err_file = strdup($2);
                g_current_command->append_out = true;
                g_current_command->append_err = true;
            }
        }
    ;

background_opt:
      /* Empty */
    | AMPERSAND
        {
            g_current_command->background = true;
        }
    ;

%%

void yyerror(const char * s)
{
    fprintf(stderr, "%s\n", s);
}

