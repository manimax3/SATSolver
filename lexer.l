%{
#include "parser.hpp"
#include <string>
#define YY_DECL \
	yy::parser::symbol_type yylex()
YY_DECL;
%}

%%
"->" {return yy::parser::make_IMPLICATION();}
"<->" {return yy::parser::make_BIIMPLICATION();}
"and" {return yy::parser::make_AND();}
"or" {return yy::parser::make_OR();}
"¬"|"not" {return yy::parser::make_NEGATION();}

"true"|"tt" { return yy::parser::make_TRUE(); }
"false"|"ff" { return yy::parser::make_FALSE(); }

"SET"|"set" { return yy::parser::make_SET(); }
"PRINT"|"print" {return yy::parser::make_PRINT(); }

[a-zA-Z]([a-zA-Z0-9])* { return yy::parser::make_PREDICATE(std::string(yytext));}
[ \t\n] {;}
. { return *yytext;}
%%

int yywrap() { return 1;}

