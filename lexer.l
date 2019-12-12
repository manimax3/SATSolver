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
(?i:and) {return yy::parser::make_AND();}
(?i:or) {return yy::parser::make_OR();}
"¬"|(?i:not) {return yy::parser::make_NEGATION();}

(?i:true)|(?i:tt) { return yy::parser::make_TRUE(); }
(?i:false)|(?i:ff) { return yy::parser::make_FALSE(); }

(?i:set) { return yy::parser::make_SET(); }
(?i:print) {return yy::parser::make_PRINT(); }
(?i:atoms) { return yy::parser::make_ATOMS(); }
(?i:table) { return yy::parser::make_TABLE(); }
(?i:nnf) { return yy::parser::make_NNF(); }
(?i:knf) { return yy::parser::make_KNF(); }

[a-zA-Z]([a-zA-Z0-9])* { return yy::parser::make_PREDICATE(std::string(yytext));}
[ \t\n] { ; }
. { return *yytext;}
%%

int yywrap() { return 1;}

