%code requires
{

}

%union
{
  char * string;
}

%token <string> WORD PIPE
%token NOTOKEN NEWLINE STDOUT STDIN STDERR STDOUT_STDERR APPEND_STDOUT APPEND_STDOUT_STDERR BACKGROUND

%{

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <dirent.h>
#include <regex.h>

#include "command.h"
#include "single_command.h"
#include "shell.h"

void yyerror(const char * s);
int yylex();

#define MAXLEN 1024

int max_entries = 20;
int num_entries = 0;
char ** result;

void expand_wild_cards(char * prefix, char * suffix);
void check_wild_cards(char * arg);
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
     single_command_list io_modifier_list background NEWLINE {
       execute_command(g_current_command);
       g_current_command = NULL;
     }
  |  NEWLINE
  ;

single_command_list:
     single_command_list PIPE single_command
  |  single_command
  ;

single_command:
     executable argument_list {
       insert_single_command(g_current_command, g_current_single_command);
       g_current_single_command = NULL;
     }
  ;

argument_list:
     argument_list argument
  |  /* can be empty */
  ;

argument:
     WORD {
       if (strcmp(g_current_single_command->arguments[0], "echo") == 0 && strchr($1, '?')) {
         insert_argument(g_current_single_command, strdup($1));
       }
       else {
         check_wild_cards(strdup($1));
       }
     }
  ;

executable:
     WORD {
       if (g_current_command == NULL) {
         g_current_command = malloc(sizeof(command_t));
         create_command(g_current_command);
       }
       g_current_single_command = malloc(sizeof(single_command_t));
       create_single_command(g_current_single_command);
       insert_argument(g_current_single_command, strdup($1));
       g_current_single_command->executable = strdup($1);
     }
  ;

io_modifier_list:
     io_modifier_list io_modifier
  |  /* can be empty */
  ;

io_modifier:
     STDOUT WORD {
       if((g_current_command->out_file) == NULL) {
         g_current_command->out_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }
     }
  |  STDIN WORD {
       if ((g_current_command->in_file) == NULL) {
         g_current_command->in_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }
     }
  |  STDERR WORD {
       if ((g_current_command->err_file) == NULL) {
         g_current_command->err_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }
     }
  |  STDOUT_STDERR WORD {
       if((g_current_command->out_file) == NULL) {
         g_current_command->out_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }

       if ((g_current_command->err_file) == NULL) {
         g_current_command->err_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }
     }
  |  APPEND_STDOUT WORD {
       if(g_current_command->out_file == NULL) {
         g_current_command->out_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }
       g_current_command->append_out = 1;
     }
  |  APPEND_STDOUT_STDERR WORD {
       if((g_current_command->out_file) == NULL) {
         g_current_command->out_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }

       if ((g_current_command->err_file) == NULL) {
         g_current_command->err_file = strdup($2);
       }
       else {
         fprintf(stderr, "Ambiguous output redirect.\n");
         exit(1);
       }
       g_current_command->append_out = 1;
       g_current_command->append_out = 1;
     }
  ;

background:
     BACKGROUND {
       g_current_command->background = 1;
     }
  |
  ;

%%

int compare(const void *file1, const void *file2) {
  const char *file1_string = *(const char **)file1;
  const char *file2_string = *(const char **)file2;
  return strcmp(file1_string, file2_string);
}

void check_wild_cards(char * arg) {
  max_entries = 20;
  num_entries = 0;
  result = malloc(max_entries * sizeof(char *));

  if (!strchr(arg, '*') && !strchr(arg, '?')) {
    insert_argument(g_current_single_command, arg);
  }
  else {
    expand_wild_cards((char *)"", arg);
    qsort(result, num_entries, sizeof(char *), compare);
    for (int i = 0; i < num_entries; i++) {
      insert_argument(g_current_single_command, result[i]);
    }
  }

  return;
}

void expand_wild_cards(char * prefix, char * suffix) {
  if (suffix[0] != '\0') {
    char * slash_pos = strchr(suffix, '/');
    char token[MAXLEN];

    if(!slash_pos) {
      strcpy(token, suffix);
      suffix = suffix + strlen(suffix);
    }
    else {
      if(!(slash_pos - suffix)) {
        strcpy(token, "");
      } else {
        strncpy(token, suffix, slash_pos - suffix);
      }
      suffix = slash_pos + 1;
    }

    char new_prefix[MAXLEN];

    if(!strchr(token, '*') && !strchr(token, '?')) {
      if(strlen(prefix) == 1 && prefix[0] == '/') {
        sprintf(new_prefix, "%s%s", prefix, token);
      }
      else {
        sprintf(new_prefix, "%s/%s", prefix, token);
      }
      expand_wild_cards(new_prefix, suffix);
      return;
    }

    char * regex = malloc(2 * strlen(token) + 3);
    char * arg_pos = token;
    char * regex_pos = regex;

    *regex_pos++ = '^';
    while (*arg_pos) {
      if (*arg_pos == '?') {
        *regex_pos++ ='.';
      }
      else if (*arg_pos == '*') {
        *regex_pos++ = '.';
        *regex_pos++ = '*';
      }
      else if (*regex_pos == '.') {
        *regex_pos++ = '\\';
        *regex_pos++ = '.';
      }
      else {
        *regex_pos++ = *arg_pos;
      }
      arg_pos++;
    }
    *regex_pos++ = '$';
    *regex_pos = '\0';

    regex_t re;
    int status = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB);
    if (status != 0) {
      perror("compile");
      return;
    }
    
    char * dir_path;
    if (prefix[0] =='\0') {
      dir_path = (char*)".";
    }
    else {
      dir_path = prefix;
    }
    
    DIR * dir = opendir(dir_path);
    if (dir == NULL) {
      if (suffix == NULL) {
        if (num_entries == max_entries) {
          max_entries *= 2;
          result = realloc(result, sizeof(char*) * max_entries);
        }
        result[num_entries] = strdup(prefix);
        num_entries += 1;
      }
      return;
    }

    struct dirent * dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
      if (!regexec(&re, dir_entry->d_name, 0, 0, 0)) {
        if (token[0] != '.' && dir_entry->d_name[0] != '.') {
          if (strlen(prefix) != 0) {
            if (strlen(prefix) == 1 && prefix[0]=='/') {
              sprintf(new_prefix, "%s%s", prefix, dir_entry->d_name);
            }
            else {
              sprintf(new_prefix, "%s/%s", prefix, dir_entry->d_name);
            }
          }
          else {
            sprintf(new_prefix, "%s", dir_entry->d_name);
          }
          expand_wild_cards(new_prefix, suffix);
        }
        else if (token[0] == '.' && dir_entry->d_name[0] == '.') {
          if (strlen(prefix) != 0) {
            sprintf(new_prefix, "%s/%s", prefix, dir_entry->d_name);
          }
          else {
            sprintf(new_prefix, "%s", dir_entry->d_name);
          }
          expand_wild_cards(new_prefix, suffix);
        }
      }
    }
    
    closedir(dir);
    regfree(&re);
  }
  else {
    if(num_entries == max_entries) {
      max_entries = max_entries * 2;
      result = realloc(result, sizeof(char *)* max_entries);
    }
    
    result[num_entries] = strdup(prefix);
    num_entries++;
  }
}

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
#endif

