/*
 * shell.l: Lexical analyzer for shell
 * Updated to support a more complex grammar
 */



%{
#include "y.tab.h"
#include <string.h>

void yyerror(const char * s);
%}

%option noyywrap

%%

[ \t]+                ; /* Skip whitespace */

\n                    { return NEWLINE; }

"2>"                  { return TWOGREAT; }
">>&"                 { return GREATGREATAMPERSAND; }
">>"                  { return GREATGREAT; }
">&"                  { return GREATAMPERSAND; }
">"                   { return GREAT; }
"<"                   { return LESS; }
"|"                   { return PIPE; }
"&"                   { return AMPERSAND; }

[^ \t\n|&><2]+        {
                        yylval.string = strdup(yytext);
                        return WORD;
                      }

.                     { /* Ignore any other character */ }

%%

