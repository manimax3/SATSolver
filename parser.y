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

%right IMPLICATION BIIMPLICATION
%left OR
%left AND
%precedence NEGATION

%type <expression> expression

%destructor {
	printf("Destruction expression\n");
	$$->print();
	printf("\n");
	delete ($$);
} <expression>

%destructor {
	delete $$;
} <pred>

%start expression

%%
expression : PREDICATE {$$ = new PredExpression(*$1); delete $1;}
		   | expression IMPLICATION expression {$$ = new ImplExpression($1, $3);}
		   | expression BIIMPLICATION expression {$$ = new BiImplExpression($1, $3);}
		   | expression AND expression {$$ = new AndExpression($1, $3);}
		   | expression OR expression {$$ = new OrExpression($1, $3);}
		   | NEGATION expression {$$ = new NegExpression($2);}
		   | '(' expression ')' { $$ = $2; }
%%

void yyerror(char *s) {
printf("Error: %s\n", s);
}

