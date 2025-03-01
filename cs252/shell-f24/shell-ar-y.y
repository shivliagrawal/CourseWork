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
  entire_command_list
  ;

entire_command_list:
     entire_command_list entire_command
  |  entire_command
      {
      }
  ;

entire_command:
     single_command_list io_modifier_list NEWLINE
      {
        execute_command(g_current_command);
      }
  |  single_command_list io_modifier_list background NEWLINE
      {
        execute_command(g_current_command);
      }
  |  NEWLINE
  ;

single_command_list:
     single_command_list PIPE single_command
  |  single_command
  ;

background:
    AMPERSAND
    {
      g_current_command->background = true;
    }
  ;

single_command:
    executable argument_list
    {
        insert_single_command(g_current_command, g_current_single_command);
    }
  ;

argument_list:
     argument_list argument
  |  /* can be empty */
  ;

argument:
     WORD
      {
        expandWildcardsIfNecessary($1);
      }
  ;

executable:
     WORD
      {
        if (g_current_command == NULL){
          g_current_command = (command_t *)malloc(sizeof(command_t));
          create_command(g_current_command);
        }
        //if (g_current_single_command == NULL){
        g_current_single_command =
              (single_command_t *)malloc(sizeof(single_command_t));
        create_single_command(g_current_single_command);
        //}
        //insert_executable(g_current_single_command, $1);
        g_current_single_command->executable = $1;
        expandWildcardsIfNecessary($1);
      }
  ;

io_modifier_list:
     io_modifier_list io_modifier
  |  /* can be empty */
  ;

io_modifier:
     GREAT WORD
      {
        g_current_command->out_file = $2;
        g_current_command->out_counter += 1;
      }
     |  LESS WORD
      {
        g_current_command->in_file = $2;
        g_current_command->in_counter += 1;
      }
     |  TWOGREAT WORD
      {
        g_current_command->err_file = $2;
      }
     |  GREATAMPERSAND WORD
      {
        g_current_command->out_file = $2;
        g_current_command->err_file = $2;
      }
     |  GREATGREAT WORD
      {
        g_current_command->out_file = $2;
        g_current_command->append_out = true;
      }
  ;
