%{
#include "parser.hpp"
#include <string>
int yylex();
%}

%%
"->" {return IMPLICATION;}
"<->" {return BIIMPLICATION;}
"and" {return AND;}
"or" {return OR;}
"¬"|"not" {return NEGATION;}

[a-zA-Z]([0-9])* {yylval.pred = new std::string(yytext); return PREDICATE;}
[ \t\n] {;}
. { return *yytext;}
%%

int yywrap() { return 1;}

