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
%define api.value.automove

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

%type <std::unique_ptr<Expression>> expression
%type <Statement*> setstmt printstmt stmt
%type <StatementList> stmtlist

%start file

%%

file : stmtlist { finalstmtlist = $1; }

stmtlist : stmt { $$ = StatementList($1); }
	 | stmtlist stmt { $$ = $1; $$.add($2); }

stmt : setstmt ';'
	 | printstmt ';'
	 /* | expression ';' { $$ = static_cast<Statement*>($1);} */

setstmt : SET ':' PREDICATE TRUE { $$ = new Statement($3, std::make_unique<ConstantExpression>(true)); }
		| SET ':' PREDICATE FALSE { $$ = new Statement($3, std::make_unique<ConstantExpression>(false)); }
		| SET PREDICATE TRUE { $$ = new Statement($2, std::make_unique<ConstantExpression>(false)); }
		| SET PREDICATE FALSE { $$ = new Statement($2, std::make_unique<ConstantExpression>(false)); }
		| SET PREDICATE ':' expression { $$ = new Statement($2, $4); }

printstmt : PRINT ':' expression { $$ = new Statement($3); }
		  | PRINT expression { $$ = new Statement($2); }
		  | PRINT ATOMS expression { $$ = new Statement($3, Statement::PrintAtoms); }
		  | PRINT TABLE expression { $$ = new Statement($3, Statement::PrintTable); }

expression : PREDICATE {$$ = std::make_unique<PredExpression>($1); }
		   | TRUE { $$ = std::make_unique<ConstantExpression>(true); }
		   | FALSE { $$ = std::make_unique<ConstantExpression>(false); }
		   | expression IMPLICATION expression {$$ = std::make_unique<BinaryExpression>($1, $3, BinaryExpression::Impl);}
		   | expression BIIMPLICATION expression {$$ = std::make_unique<BinaryExpression>($1, $3, BinaryExpression::BiImpl);}
		   | expression AND expression {$$ = std::make_unique<BinaryExpression>($1, $3, BinaryExpression::And);}
		   | expression OR expression {$$ = std::make_unique<BinaryExpression>($1, $3, BinaryExpression::Or);}
		   | NEGATION expression {$$ = std::make_unique<NegExpression>($2);}
		   | '(' expression ')' { $$ = $2; }
%%

void yyerror(char *s) {
printf("Error: %s\n", s);
}

void yy::parser::error(const std::string& error)
{
	std::cerr << error << '\n';
}


