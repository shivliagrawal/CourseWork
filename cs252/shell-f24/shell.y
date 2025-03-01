/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{

}

%union
{
  char * string;
}

%token <string> WORD PIPE
%token NOTOKEN NEWLINE STDOUT STDIN STDERR STDOUT_STDERR APPEND_STDOUT APPEND_STDOUTERR BACKGROUND

%{

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "command.h"
#include "single_command.h"
#include "shell.h"

void yyerror(const char * s);
int yylex();

%}

%%

goal:
  entire_command_list
  ;

entire_command_list:
     entire_command_list entire_command
  |  entire_command
  ;

entire_command:
    /* single_command_list io_modifier_list NEWLINE {
    /* if (g_current_command == NULL) {
        g_current_command = (command_t *)malloc(sizeof(command_t));
     }
       execute_command(g_current_command);
       g_current_command = NULL;

     }
  |*/
  single_command_list io_modifier_list background NEWLINE {
        /* free happens inside this function */
        execute_command(g_current_command);
        g_current_command = NULL;

    }
  | NEWLINE
  ;

background:
  BACKGROUND {
    g_current_command->background = true;
  }
  |
  ;
single_command_list:
     single_command_list PIPE single_command
  |  single_command
  ;

single_command:
    executable argument_list {
      /* insert new single_command into current command structure*/
      insert_single_command(g_current_command, g_current_single_command);

      //printf("Inserted command: %s\n", g_current_single_command->executable);
      g_current_single_command = NULL;

    }
  ;

argument_list:
     argument_list argument
  |  /* can be empty */
  ;

argument:
     WORD {
      /* first token that is parsed (ls -l) is -l */
      insert_argument(g_current_single_command, $1);
     }
  ;

executable:
     WORD { // (ls -l) would be ls
      if (g_current_command == NULL) {
        g_current_command = (command_t *)malloc(sizeof(command_t));
        create_command(g_current_command);
      }
      //create_command(g_current_command);
      g_current_single_command = (single_command_t *)malloc(sizeof(single_command_t));
      create_single_command(g_current_single_command);
      // argument list is null

      insert_argument(g_current_single_command, $1);
      g_current_single_command->executable = $1;

     }
  ;

io_modifier_list:
     io_modifier_list io_modifier
  |  /* can be empty */   
  ;

io_modifier:
     STDOUT WORD {
      if((g_current_command->out_file) == NULL) {
       // g_current_command->out_file = $2;
        g_current_command->out_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->out_file, $2);

      }
      else {
        fprintf(stderr, "Ambiguous output redirect.\n");

      }

     }
    | STDIN WORD {
      if ((g_current_command->in_file) == NULL) {
       // g_current_command->in_file = $2;
        g_current_command->in_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->in_file, $2);
      }
      else {
        fprintf(stderr, "Ambiguous output redirect.\n");

      }

    }
    | STDERR WORD {
      if ((g_current_command->err_file) == NULL) {
        //g_current_command->err_file = $2;
        g_current_command->err_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->err_file, $2);

      }
      else {
        fprintf(stderr, "Ambiguous output redirect.\n");

      }

    }
    | STDOUT_STDERR WORD {
      if ((g_current_command->out_file) == NULL) {
        //g_current_command->out_file = $2;
        g_current_command->out_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->out_file, $2);

      }
      else {
        fprintf(stderr, "Ambiguous output redirect.\n");
      }
      if ((g_current_command->err_file) == NULL) {
        //g_current_command->err_file = $2;
        g_current_command->err_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->err_file, $2);

      } // return ambig error 
      else {
        fprintf(stderr, "Ambiguous output redirect.\n");

      }

    }
    | APPEND_STDOUT WORD {
      if ((g_current_command->out_file) == NULL) {
        //g_current_command->out_file = $2;
        g_current_command->out_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->out_file, $2);
        g_current_command->append_out = true;

      }
      else {
        fprintf(stderr, "Ambiguous output redirect.\n");

      }

    }
    | APPEND_STDOUTERR WORD {
      if (((g_current_command->out_file) == NULL) ||((g_current_command->err_file) == NULL)) {
       // g_current_command->out_file = $2;
        g_current_command->out_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->out_file, $2);
        g_current_command->append_out = true;

       // g_current_command->err_file = $2;
        g_current_command->err_file = (char *)malloc(strlen($2) + 1);
        strcpy(g_current_command->err_file, $2);
        g_current_command->append_err = true;


      }
      else {
        fprintf(stderr, "Ambiguous output redirect.\n");

      }

    }

  ;


%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif/
