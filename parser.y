%{
int yylex();
#include <cstdio>
#include <cstdlib>
void yyerror(char *s);
%}

%code requires {
#include <string>
#include "ast.h"
}

%union {
	struct Expression *expression;
	std::string* pred;
}

%token IMPLICATION
%token BIIMPLICATION
%token <pred> PREDICATE
%token AND 
%token OR
%token NEGATION

%type <expression> expression

%destructor {
	printf("Destruction expression or predicate\n");
	delete ($$);
} <expression> <pred>

%start expression

%%
expression : PREDICATE AND PREDICATE {$$ = new Expression(nullptr, nullptr); 
			   printf("Found predicates %s and %s\n", $1->c_str(), $3->c_str());
			   delete $1;
			   delete $3;
		   }
%%

void yyerror(char *s) {
printf("Error: %s\n", s);
}

