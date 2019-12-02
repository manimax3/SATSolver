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
	StatementList *stmtlist;
	Statement *stmt;
	Expression *expression;
	std::string* pred;
	bool value;
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


%token TRUE
%token FALSE
%token SET

%type <expression> expression
%type <stmt> setstmt stmt
%type <stmtlist> stmtlist

%destructor {
	printf("Destruction expression\n");
	$$->print();
	printf("\n");
	delete ($$);
} <expression>

%destructor {
	delete $$;
} <pred> <stmt> 

%destructor {
	$$->print();
	delete $$;
} <stmtlist>

%start stmtlist

%%
stmtlist : stmt { $$ = new StatementList($1); }
	 | stmtlist stmt { $$ = new StatementList(*$1, $2); delete $1; }

stmt : setstmt ';'
	 | expression ';' { $$ = static_cast<Statement*>($1);}

setstmt : SET ':' PREDICATE TRUE { $$ = new SetStatement(*$3, true); delete $3; }
		| SET ':' PREDICATE FALSE { $$ = new SetStatement(*$3, false); delete $3; }

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

