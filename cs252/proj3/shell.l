/*
 *
 * 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{
#include "y.tab.h"
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>

void yyerror(const char *s);

extern int last_exit_status;
extern pid_t shell_pid;
extern pid_t last_bg_pid;
extern char * last_argument;
extern char * shell_path;
extern char * last_command;
%}

%option noyywrap

%%

[ \t]+                ; /* Skip whitespace */

\n                    { return NEWLINE; }

"2>"                  { return STDERR; }
">>&"                 { return APPEND_STDOUT_STDERR; }
">>"                  { return APPEND_STDOUT; }
">&"                  { return STDOUT_STDERR; }
">"                   { return STDOUT; }
"<"                   { return STDIN; }
"|"                   { return PIPE; }
"&"                   { return BACKGROUND; }

["][^\n\"]*["] {
  yylval.string = strdup(yytext + 1);
  if (yylval.string[yyleng - 2] == '"') {
    yylval.string[yyleng - 2] = '\0';
  }
  return WORD;
}

\$\([^\n\$]*\) {
  char * command = strdup(yytext+2);
  int k = 0;
  for (int i = 0; i < strlen(yytext); i++) {
    if (yytext[i] != '$' && yytext[i] != '(' && yytext[i] != ')') {
      strncpy(&command[k], &yytext[i], 1);
      k++;
    }
  }
  command[k] = '\0';

  int tmpin = dup(0);
  int tmpout = dup(1);

  int fdpipein[2];
  int fdpipeout[2];

  pipe(fdpipein);
  pipe(fdpipeout);

  write(fdpipein[1], command, strlen(command));
  write(fdpipein[1], "\n", 1);
  write(fdpipein[1], "exit", 4);
  write(fdpipein[1], "\n", 1);

  close(fdpipein[1]);

  dup2(fdpipein[0], 0);
  close(fdpipein[0]);
  dup2(fdpipeout[1], 1);
  close(fdpipeout[1]);

  int ret = fork();
  if (ret == 0) {
    execvp("/proc/self/exe", NULL);
    _exit(1);
  } else if (ret < 0) {
    perror("fork");
    exit(1);
  }

  dup2(tmpin, 0);
  dup2(tmpout, 1);
  close(tmpin);
  close(tmpout);

  char c;
  char * buffer = malloc(4096);
  int i = 0;

  while (read(fdpipeout[0], &c, 1)) {
    if (c == '\n') {
      buffer[i++] = ' ';
    }
    else {
      buffer[i++] = c;
    }
  }
  buffer[i] = '\0';

  for (int j = i - 1; j >= 0; j--) {
    unput(buffer[j]);
  }
}

[^ \t\n|><&]*\\[^ \t\n]* {
  char * word = malloc(strlen(yytext));
  char * c = yytext;

  int i = 0;
  while (*c) {
    if (*c == '\\') {
      if (*(c + 1) == '\\') {
        c += 2;
        word[i] = '\\';
      } else {
        ++c;
        word[i] = *(c);
      }
    } else {
      word[i] = *c;
    }
    c++;
    i++;
  }

  word[i] = '\0';
  yylval.string = strdup(word);
  return WORD;
}

~[^ \t\n]*  {
  char * argument = yytext;
  char * expanded;

  if (strcmp(argument, "~") == 0) {
    expanded = strdup(getenv("HOME"));
  }
  else if (argument[1] == '/') {
    char * dir = getenv("HOME");
    expanded = malloc(strlen(dir) + strlen(argument));
    strcpy(expanded, dir);
    strcat(expanded, argument + 1);
  }
  else {
    char * temp = argument + 1;
    char username[256];
    int i = 0;
    while (*temp != '/' && *temp && i < 255) {
      username[i++] = *temp++;
    }
    username[i] = '\0';

    struct passwd *pw = getpwnam(username);
    if (pw == NULL) {
      fprintf(stderr, "Unknown user: %s\n", username);
      exit(1);
    }

    if (*temp) {
      expanded = (char *) malloc(strlen(pw->pw_dir) + strlen(temp) + 1);
      strcpy(expanded, pw->pw_dir);
      strcat(expanded, temp);
    }
    else {
      expanded = strdup(pw->pw_dir);
    }
  }

  yylval.string = expanded;
  return WORD;
}


[^ \t\n|><][^ \t\n|><]* {
  yylval.string = strdup(yytext);
  return WORD;
}

"source "[^\t\n]* {
  char * file = yytext + 7;
  FILE * in_file = fopen(file, "r+");

  if (!in_file) {
    fprintf(stderr, "source: can not find %s", file);
    exit(2);
  }

  fseek(in_file, 0, SEEK_SET);
  yypush_buffer_state(yy_create_buffer(in_file, YY_BUF_SIZE));
  yyparse();
  yypop_buffer_state();
  fclose(in_file);
}

%%
