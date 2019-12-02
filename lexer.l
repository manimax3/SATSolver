%{
#include "parser.hpp"
int yylex();
%}

%%
"print" {printf("Found print\n"); return print;}
[0-9]+ {yylval.num = atoi(yytext); printf("Number: %i\n", yylval.num); return number;}
[ \t\n] {;}
. {ECHO; return *yytext;}
%%

int yywrap() { return 1;}

