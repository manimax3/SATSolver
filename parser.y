%{
#include <cstdio>
#include <cstdlib>
#include "ast.h"
extern StatementList finalstmtlist;
%}

%require "3.2"
%language "c++"
%define api.value.type variant
%define api.token.constructor

%code requires {
#include <string>
#include "ast.h"
}

%code provides {
yy::parser::symbol_type yylex();
}

%token IMPLICATION
%token BIIMPLICATION
%token <std::string> PREDICATE
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
%token PRINT
%token ATOMS
%token TABLE

%token EndOfFile 0

%type <Expression*> expression
%type <Statement*> setstmt printstmt stmt
%type <StatementList> stmtlist file

%start file

%%

file : stmtlist { finalstmtlist = std::move($1); $$ = std::move($1); }

stmtlist : stmt { $$ = StatementList($1); }
	 | stmtlist stmt { $$ = std::move($1); $$.add($2); }

stmt : setstmt ';'
	 | printstmt ';'
	 | expression ';' { $$ = static_cast<Statement*>($1);}

setstmt : SET ':' PREDICATE TRUE { $$ = new Statement($3, new ConstantExpression(true)); }
		| SET ':' PREDICATE FALSE { $$ = new Statement($3, new ConstantExpression(false)); }
		| SET PREDICATE TRUE { $$ = new Statement($2, new ConstantExpression(false)); }
		| SET PREDICATE FALSE { $$ = new Statement($2, new ConstantExpression(false)); }
		| SET PREDICATE ':' expression { $$ = new Statement($2, $4); }

printstmt : PRINT ':' expression { $$ = new Statement($3); }
		  | PRINT expression { $$ = new Statement($2); }
		  | PRINT ATOMS expression { $$ = new Statement($3, Statement::PrintAtoms); }
		  | PRINT TABLE expression { $$ = new Statement($3, Statement::PrintTable); }

expression : PREDICATE {$$ = new PredExpression($1); }
		   | TRUE { $$ = new ConstantExpression(true); }
		   | FALSE { $$ = new ConstantExpression(false); }
		   | expression IMPLICATION expression {$$ = new BinaryExpression($1, $3, BinaryExpression::Impl);}
		   | expression BIIMPLICATION expression {$$ = new BinaryExpression($1, $3, BinaryExpression::BiImpl);}
		   | expression AND expression {$$ = new BinaryExpression($1, $3, BinaryExpression::And);}
		   | expression OR expression {$$ = new BinaryExpression($1, $3, BinaryExpression::Or);}
		   | NEGATION expression {$$ = new NegExpression($2);}
		   | '(' expression ')' { $$ = $2; }
%%

void yyerror(char *s) {
printf("Error: %s\n", s);
}

void yy::parser::error(const std::string& error)
{
	std::cerr << error << '\n';
}


