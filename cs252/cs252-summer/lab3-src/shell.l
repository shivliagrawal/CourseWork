%{
#include "y.tab.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void push_char(int c);
char* process_escapes(const char* input);
void handle_subshell(const char* cmd);
void source_file(const char* path);

#define MAX_CMD_LEN 4096
%}

%option noyywrap

WHITESPACE  [ \t]
REDIRECT    ">"|">>"|"2>"|"<"|">&"|">>&"
CONTROL     [|&]
QUOTES      \"[^\n\"]*\"
ESCAPED     [^ \t\n|><&]*\\[^ \t\n]*
WORD        [^ \t\n<>|&]+
SUBSHELL    \$\([^\n\$]*\)
SOURCE      "source "[^\t\n]*

%%

{WHITESPACE}    { /* Ignore whitespace */ }
\n              { return NEWLINE; }

{REDIRECT}      { 
                  if (strcmp(yytext, ">") == 0) return STDOUT;
                  if (strcmp(yytext, ">>") == 0) return APPEND_STDOUT;
                  if (strcmp(yytext, "2>") == 0) return STDERR;
                  if (strcmp(yytext, "<") == 0) return STDIN;
                  if (strcmp(yytext, ">&") == 0) return STDOUT_STDERR;
                  if (strcmp(yytext, ">>&") == 0) return APPEND_STDOUT_STDERR;
                }

{CONTROL}       { return (yytext[0] == '|') ? PIPE : AMPERSAND; }

{QUOTES}        { 
                  yylval.string = strdup(yytext + 1);
                  yylval.string[yyleng - 2] = '\0';
                  return WORD;
                }

{ESCAPED}       {
                  yylval.string = process_escapes(yytext);
                  return WORD;
                }

{WORD}          { 
                  yylval.string = strdup(yytext);
                  return WORD;
                }

{SUBSHELL}      { handle_subshell(yytext); }

{SOURCE}        { source_file(yytext + 7); }

%%

void push_char(int c) {
    unput(c);
}

char* process_escapes(const char* input) {
    char* output = malloc(strlen(input) + 1);
    int i = 0, j = 0;
    
    while (input[i]) {
        if (input[i] == '\\' && input[i+1]) {
            output[j++] = input[i+1];
            i += 2;
        } else {
            output[j++] = input[i++];
        }
    }
    output[j] = '\0';
    return output;
}

void handle_subshell(const char* cmd) {
    char buffer[MAX_CMD_LEN] = {0};
    int fds[2];
    pid_t pid;

    pipe(fds);
    pid = fork();

    if (pid == 0) {
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);
        
        execl("/proc/self/exe", "/proc/self/exe", NULL);
        perror("execl");
        exit(1);
    } else if (pid > 0) {
        close(fds[1]);
        int n = read(fds[0], buffer, sizeof(buffer) - 1);
        close(fds[0]);
        waitpid(pid, NULL, 0);

        for (int i = n - 1; i >= 0; i--) {
            push_char(buffer[i] == '\n' ? ' ' : buffer[i]);
        }
    } else {
        perror("fork");
        exit(1);
    }
}

void source_file(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "source: cannot open %s\n", path);
        exit(2);
    }

    YY_BUFFER_STATE prev_buffer = YY_CURRENT_BUFFER;
    yy_switch_to_buffer(yy_create_buffer(file, YY_BUF_SIZE));
    yyparse();
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(prev_buffer);
    fclose(file);
}
