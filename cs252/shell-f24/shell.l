/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{
#include "y.tab.h"
#include <string.h>

void yyerror(const char *s);
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

[^ \t\n|><][^ \t\n|><]*        {
  yylval.string = strdup(yytext);
  return WORD;
}

.                     { /* Ignore any other character */ }

%%
