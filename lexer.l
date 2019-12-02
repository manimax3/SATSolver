%{
#include "parser.hpp"
#include <string>
int yylex();
%}

%%
"impl" {return IMPLICATION;}
"and" {return AND;}
[a-zA-Z]([0-9])* {yylval.pred = new std::string(yytext); return PREDICATE;}
[ \t\n] {;}
. {ECHO; return *yytext;}
%%

int yywrap() { return 1;}

